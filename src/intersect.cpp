#include "mse/intersect.hpp"

#include <algorithm>
#include <cmath>

namespace mse {

std::vector<DocId> posting_docs(const std::vector<Posting>& postings) {
    std::vector<DocId> out;
    out.reserve(postings.size());
    for (const auto& p : postings)
        out.push_back(p.doc_id);
    return out;
}

std::vector<DocId> intersect_two_pointer(const std::vector<DocId>& a, const std::vector<DocId>& b) {
    std::vector<DocId> out;
    out.reserve(std::min(a.size(), b.size()));
    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            out.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return out;
}

namespace {

// Exponential search for first element >= target starting at index lo.
std::size_t gallop_find(const std::vector<DocId>& v, std::size_t lo, DocId target) {
    if (lo >= v.size() || v[lo] >= target)
        return lo;
    std::size_t step = 1;
    std::size_t hi = lo + 1;
    while (hi < v.size() && v[hi] < target) {
        lo = hi;
        hi = lo + step;
        step <<= 1;
        if (hi > v.size())
            hi = v.size();
    }
    // Binary search in [lo, hi)
    auto it = std::lower_bound(v.begin() + static_cast<std::ptrdiff_t>(lo),
                               v.begin() + static_cast<std::ptrdiff_t>(hi), target);
    return static_cast<std::size_t>(it - v.begin());
}

} // namespace

std::vector<DocId> intersect_galloping(const std::vector<DocId>& a, const std::vector<DocId>& b) {
    // Iterate the shorter list; gallop on the longer.
    const std::vector<DocId>* short_l = &a;
    const std::vector<DocId>* long_l = &b;
    if (a.size() > b.size()) {
        short_l = &b;
        long_l = &a;
    }
    std::vector<DocId> out;
    out.reserve(short_l->size());
    std::size_t j = 0;
    for (DocId x : *short_l) {
        j = gallop_find(*long_l, j, x);
        if (j >= long_l->size())
            break;
        if ((*long_l)[j] == x) {
            out.push_back(x);
            ++j;
        }
    }
    return out;
}

SkipList make_skip_list(std::vector<DocId> docs) {
    SkipList sl;
    sl.docs = std::move(docs);
    if (sl.docs.empty()) {
        sl.interval = 1;
        return sl;
    }
    // interval ≈ √n (at least 1)
    sl.interval = static_cast<std::size_t>(std::sqrt(static_cast<double>(sl.docs.size())));
    if (sl.interval < 1)
        sl.interval = 1;
    for (std::size_t i = 0; i < sl.docs.size(); i += sl.interval) {
        const std::size_t dest = std::min(i + sl.interval, sl.docs.size());
        sl.skip_to.push_back(dest);
        sl.skip_doc.push_back(dest < sl.docs.size() ? sl.docs[dest] : sl.docs.back() + 1);
    }
    return sl;
}

namespace {

// Advance cursor on skip list until docs[cursor] >= target (or end).
std::size_t skip_advance(const SkipList& list, std::size_t cursor, DocId target) {
    const std::size_t n = list.docs.size();
    while (cursor < n && list.docs[cursor] < target) {
        if (list.interval > 0 && cursor % list.interval == 0) {
            const std::size_t block = cursor / list.interval;
            if (block < list.skip_to.size()) {
                const std::size_t dest = list.skip_to[block];
                if (dest < n && list.docs[dest] <= target) {
                    cursor = dest;
                    continue;
                }
            }
        }
        ++cursor;
    }
    return cursor;
}

} // namespace

std::vector<DocId> intersect_skip_pointers(const SkipList& a, const SkipList& b) {
    std::vector<DocId> out;
    std::size_t i = 0, j = 0;
    while (i < a.docs.size() && j < b.docs.size()) {
        if (a.docs[i] == b.docs[j]) {
            out.push_back(a.docs[i]);
            ++i;
            ++j;
        } else if (a.docs[i] < b.docs[j]) {
            i = skip_advance(a, i, b.docs[j]);
        } else {
            j = skip_advance(b, j, a.docs[i]);
        }
    }
    return out;
}

std::vector<DocId> intersect_with_skips(const std::vector<DocId>& a, const std::vector<DocId>& b) {
    return intersect_skip_pointers(make_skip_list(a), make_skip_list(b));
}

std::vector<DocId> unite(const std::vector<DocId>& a, const std::vector<DocId>& b) {
    std::vector<DocId> out;
    out.reserve(a.size() + b.size());
    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            out.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            out.push_back(a[i++]);
        } else {
            out.push_back(b[j++]);
        }
    }
    while (i < a.size())
        out.push_back(a[i++]);
    while (j < b.size())
        out.push_back(b[j++]);
    return out;
}

std::vector<DocId> difference(const std::vector<DocId>& a, const std::vector<DocId>& b) {
    std::vector<DocId> out;
    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            out.push_back(a[i++]);
        } else {
            ++j;
        }
    }
    while (i < a.size())
        out.push_back(a[i++]);
    return out;
}

} // namespace mse

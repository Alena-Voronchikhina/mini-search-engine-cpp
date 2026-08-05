#include "mse/query_eval.hpp"

#include "mse/intersect.hpp"

#include <algorithm>

namespace mse {
namespace {

std::vector<DocId> intersect(const std::vector<DocId>& a, const std::vector<DocId>& b,
                             IntersectMode mode) {
    if (mode == IntersectMode::TwoPointer)
        return intersect_two_pointer(a, b);
    if (mode == IntersectMode::SkipPointers)
        return intersect_with_skips(a, b);
    return intersect_galloping(a, b);
}

std::vector<DocId> term_docs(const Index& index, const std::string& term) {
    const auto* p = index.postings(term);
    if (!p)
        return {};
    return posting_docs(*p);
}

std::vector<DocId> phrase_docs(const Index& index, const std::vector<std::string>& terms) {
    if (terms.empty())
        return {};
    const auto* first = index.postings(terms[0]);
    if (!first)
        return {};
    std::vector<DocId> candidates = posting_docs(*first);
    for (std::size_t t = 1; t < terms.size(); ++t) {
        const auto* p = index.postings(terms[t]);
        if (!p)
            return {};
        candidates = intersect_galloping(candidates, posting_docs(*p));
        if (candidates.empty())
            return {};
    }
    std::vector<DocId> out;
    for (DocId d : candidates) {
        if (phrase_matches(index, terms, d))
            out.push_back(d);
    }
    return out;
}

std::vector<DocId> eval(const Index& index, const QueryNode& node, IntersectMode mode) {
    switch (node.kind) {
    case NodeKind::Term:
        return term_docs(index, node.term);
    case NodeKind::Phrase:
        return phrase_docs(index, node.phrase_terms);
    case NodeKind::And: {
        auto L = eval(index, *node.left, mode);
        auto R = eval(index, *node.right, mode);
        return intersect(L, R, mode);
    }
    case NodeKind::Or: {
        auto L = eval(index, *node.left, mode);
        auto R = eval(index, *node.right, mode);
        return unite(L, R);
    }
    case NodeKind::Not: {
        auto child = eval(index, *node.child, mode);
        return difference(index.all_doc_ids(), child);
    }
    }
    return {};
}

} // namespace

bool phrase_matches(const Index& index, const std::vector<std::string>& terms, DocId doc) {
    if (terms.empty())
        return false;
    const auto* first_pl = index.postings(terms[0]);
    if (!first_pl)
        return false;
    auto it0 = std::lower_bound(first_pl->begin(), first_pl->end(), doc,
                                [](const Posting& p, DocId d) { return p.doc_id < d; });
    if (it0 == first_pl->end() || it0->doc_id != doc)
        return false;

    for (Pos start : it0->positions) {
        bool ok = true;
        for (std::size_t i = 1; i < terms.size(); ++i) {
            const auto* pl = index.postings(terms[i]);
            if (!pl) {
                ok = false;
                break;
            }
            auto it = std::lower_bound(pl->begin(), pl->end(), doc,
                                       [](const Posting& p, DocId d) { return p.doc_id < d; });
            if (it == pl->end() || it->doc_id != doc) {
                ok = false;
                break;
            }
            const Pos need = start + static_cast<Pos>(i);
            if (!std::binary_search(it->positions.begin(), it->positions.end(), need)) {
                ok = false;
                break;
            }
        }
        if (ok)
            return true;
    }
    return false;
}

std::vector<DocId> evaluate_boolean(const Index& index, const QueryNode& root, IntersectMode mode) {
    return eval(index, root, mode);
}

} // namespace mse

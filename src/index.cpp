#include "mse/index.hpp"

#include <algorithm>

namespace mse {

void Index::add_document(std::string path, const std::vector<std::string>& tokens) {
    const DocId id = static_cast<DocId>(docs_.size());
    docs_.push_back(DocumentMeta{std::move(path), static_cast<std::uint32_t>(tokens.size())});

    for (Pos pos = 0; pos < static_cast<Pos>(tokens.size()); ++pos) {
        const std::string& term = tokens[pos];
        auto& plist = inv_[term];
        if (plist.empty() || plist.back().doc_id != id) {
            plist.push_back(Posting{id, {}});
        }
        plist.back().positions.push_back(pos);
    }
    finalized_ = false;
}

void Index::finalize() {
    for (auto& [_, plist] : inv_) {
        std::sort(plist.begin(), plist.end(),
                  [](const Posting& a, const Posting& b) { return a.doc_id < b.doc_id; });
    }
    std::uint64_t total = 0;
    for (const auto& d : docs_)
        total += d.length;
    avgdl_ = docs_.empty() ? 0.0 : static_cast<double>(total) / static_cast<double>(docs_.size());
    finalized_ = true;
}

const std::vector<Posting>* Index::postings(const std::string& term) const {
    auto it = inv_.find(term);
    if (it == inv_.end())
        return nullptr;
    return &it->second;
}

std::uint32_t Index::df(const std::string& term) const {
    const auto* p = postings(term);
    return p ? static_cast<std::uint32_t>(p->size()) : 0;
}

std::uint32_t Index::tf(const std::string& term, DocId doc) const {
    const auto* p = postings(term);
    if (!p)
        return 0;
    auto it = std::lower_bound(p->begin(), p->end(), doc,
                               [](const Posting& post, DocId d) { return post.doc_id < d; });
    if (it == p->end() || it->doc_id != doc)
        return 0;
    return static_cast<std::uint32_t>(it->positions.size());
}

std::vector<DocId> Index::all_doc_ids() const {
    std::vector<DocId> ids(docs_.size());
    for (DocId i = 0; i < static_cast<DocId>(docs_.size()); ++i)
        ids[i] = i;
    return ids;
}

} // namespace mse

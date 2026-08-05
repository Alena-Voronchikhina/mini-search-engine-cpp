#include "mse/ranker.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_set>

namespace mse {

std::vector<std::string> collect_query_terms(const QueryNode& root) {
    std::vector<std::string> out;
    std::function<void(const QueryNode&)> walk = [&](const QueryNode& n) {
        switch (n.kind) {
        case NodeKind::Term:
            out.push_back(n.term);
            break;
        case NodeKind::Phrase:
            out.insert(out.end(), n.phrase_terms.begin(), n.phrase_terms.end());
            break;
        case NodeKind::Not:
            if (n.child)
                walk(*n.child);
            break;
        case NodeKind::And:
        case NodeKind::Or:
            if (n.left)
                walk(*n.left);
            if (n.right)
                walk(*n.right);
            break;
        }
    };
    walk(root);
    return out;
}

std::vector<RankedHit> rank_bm25(const Index& index, const std::vector<std::string>& terms,
                                 std::size_t topk, const std::vector<DocId>& candidates,
                                 Bm25Params params) {
    if (terms.empty() || topk == 0 || index.num_docs() == 0)
        return {};

    std::vector<DocId> docs = candidates;
    if (docs.empty()) {
        std::unordered_set<DocId> seen;
        for (const auto& t : terms) {
            const auto* p = index.postings(t);
            if (!p)
                continue;
            for (const auto& post : *p) {
                if (seen.insert(post.doc_id).second)
                    docs.push_back(post.doc_id);
            }
        }
        std::sort(docs.begin(), docs.end());
    }

    const double N = static_cast<double>(index.num_docs());
    const double avgdl = index.avgdl();

    auto score_doc = [&](DocId d) {
        double score = 0.0;
        const double dl = static_cast<double>(index.documents()[d].length);
        for (const auto& t : terms) {
            const auto tf = static_cast<double>(index.tf(t, d));
            if (tf == 0.0)
                continue;
            const auto df = static_cast<double>(index.df(t));
            const double idf = std::log1p((N - df + 0.5) / (df + 0.5));
            const double denom =
                tf + params.k1 * (1.0 - params.b + params.b * (dl / std::max(avgdl, 1e-9)));
            score += idf * (tf * (params.k1 + 1.0) / denom);
        }
        return score;
    };

    using QItem = std::pair<double, DocId>; // min-heap by score
    std::priority_queue<QItem, std::vector<QItem>, std::greater<>> heap;
    for (DocId d : docs) {
        const double s = score_doc(d);
        if (s <= 0.0)
            continue;
        if (heap.size() < topk) {
            heap.emplace(s, d);
        } else if (s > heap.top().first) {
            heap.pop();
            heap.emplace(s, d);
        }
    }

    std::vector<RankedHit> out;
    out.reserve(heap.size());
    while (!heap.empty()) {
        out.push_back(RankedHit{heap.top().second, heap.top().first});
        heap.pop();
    }
    std::reverse(out.begin(), out.end());
    return out;
}

} // namespace mse

#pragma once

#include "mse/index.hpp"
#include "mse/query_ast.hpp"
#include "mse/types.hpp"

#include <cstddef>
#include <vector>

namespace mse {

struct Bm25Params {
    double k1{1.2};
    double b{0.75};
};

// Collect leaf terms (and phrase terms) from AST for BM25 scoring.
[[nodiscard]] std::vector<std::string> collect_query_terms(const QueryNode& root);

// If candidates is empty, score over documents that contain at least one term.
[[nodiscard]] std::vector<RankedHit> rank_bm25(const Index& index,
                                               const std::vector<std::string>& terms,
                                               std::size_t topk,
                                               const std::vector<DocId>& candidates = {},
                                               Bm25Params params = {});

} // namespace mse

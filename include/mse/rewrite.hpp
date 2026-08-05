#pragma once

#include "mse/query_ast.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mse {

// synonym file lines: "car,automobile,vehicle" (comma-separated, case-insensitive keys)
using SynonymMap = std::unordered_map<std::string, std::vector<std::string>>;

[[nodiscard]] SynonymMap load_synonyms(const std::string& path);

// Expand each Term node into OR of itself + synonyms (recursively).
[[nodiscard]] std::unique_ptr<QueryNode> rewrite_synonyms(const QueryNode& root,
                                                          const SynonymMap& syns);

} // namespace mse

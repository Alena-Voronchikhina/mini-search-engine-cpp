#pragma once

#include "mse/index.hpp"
#include "mse/query_ast.hpp"
#include "mse/types.hpp"

#include <vector>

namespace mse {

enum class IntersectMode { TwoPointer, Galloping, SkipPointers };

[[nodiscard]] std::vector<DocId> evaluate_boolean(const Index &index, const QueryNode &root,
                                                  IntersectMode mode = IntersectMode::Galloping);

[[nodiscard]] bool phrase_matches(const Index &index, const std::vector<std::string> &terms,
                                  DocId doc);

} // namespace mse

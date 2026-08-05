#pragma once

#include "mse/types.hpp"

#include <vector>

namespace mse {

// Sorted ascending unique DocId lists.
[[nodiscard]] std::vector<DocId> intersect_two_pointer(const std::vector<DocId>& a,
                                                     const std::vector<DocId>& b);

[[nodiscard]] std::vector<DocId> intersect_galloping(const std::vector<DocId>& a,
                                                     const std::vector<DocId>& b);

[[nodiscard]] std::vector<DocId> unite(const std::vector<DocId>& a, const std::vector<DocId>& b);

[[nodiscard]] std::vector<DocId> difference(const std::vector<DocId>& a,
                                            const std::vector<DocId>& b);

[[nodiscard]] std::vector<DocId> posting_docs(const std::vector<Posting>& postings);

} // namespace mse

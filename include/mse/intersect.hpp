#pragma once

#include "mse/types.hpp"

#include <cstddef>
#include <vector>

namespace mse {

// Sorted ascending unique DocId lists.
[[nodiscard]] std::vector<DocId> intersect_two_pointer(const std::vector<DocId> &a,
                                                       const std::vector<DocId> &b);

[[nodiscard]] std::vector<DocId> intersect_galloping(const std::vector<DocId> &a,
                                                     const std::vector<DocId> &b);

// Classic IR skip pointers: every ~sqrt(n) entry stores a forward jump target.
struct SkipList {
    std::vector<DocId> docs;
    std::size_t interval{1};
    // For index i where i % interval == 0: skip_to[i / interval] is the index to jump to
    // (still may need sequential scan within the block).
    std::vector<std::size_t> skip_to;
    std::vector<DocId> skip_doc; // docs[skip_to[k]]
};

[[nodiscard]] SkipList make_skip_list(std::vector<DocId> docs);

[[nodiscard]] std::vector<DocId> intersect_skip_pointers(const SkipList &a, const SkipList &b);

// Convenience: build skip lists then intersect (same results as two-pointer).
[[nodiscard]] std::vector<DocId> intersect_with_skips(const std::vector<DocId> &a,
                                                      const std::vector<DocId> &b);

[[nodiscard]] std::vector<DocId> unite(const std::vector<DocId> &a, const std::vector<DocId> &b);

[[nodiscard]] std::vector<DocId> difference(const std::vector<DocId> &a,
                                            const std::vector<DocId> &b);

[[nodiscard]] std::vector<DocId> posting_docs(const std::vector<Posting> &postings);

} // namespace mse

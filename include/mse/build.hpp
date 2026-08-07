#pragma once

#include "mse/index.hpp"
#include "mse/tokenizer.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace mse {

struct BuildOptions {
    TokenizerOptions tokenizer{};
    std::size_t threads{1}; // 1 = serial; 0 = hardware_concurrency
};

// Index all .txt/.md files under `dir` (recursive). Uses threads>1 for parallel shard build+merge.
[[nodiscard]] Index build_index_from_dir(const std::filesystem::path &dir, BuildOptions opts = {});

} // namespace mse

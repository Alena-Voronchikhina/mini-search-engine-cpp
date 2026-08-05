#pragma once

#include "mse/index.hpp"

#include <string>

namespace mse {

// Load index by memory-mapping the file (falls back to buffered read if mmap unavailable).
[[nodiscard]] bool load_index_mmap(Index& index, const std::string& path);

} // namespace mse

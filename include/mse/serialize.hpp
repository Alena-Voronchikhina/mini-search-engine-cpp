#pragma once

#include "mse/index.hpp"

#include <string>

namespace mse {

[[nodiscard]] bool save_index(const Index& index, const std::string& path);
[[nodiscard]] bool load_index(Index& index, const std::string& path);

} // namespace mse

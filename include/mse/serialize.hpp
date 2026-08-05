#pragma once

#include "mse/index.hpp"

#include <cstdint>
#include <string>

namespace mse {

[[nodiscard]] bool save_index(const Index& index, const std::string& path);
[[nodiscard]] bool load_index(Index& index, const std::string& path);
[[nodiscard]] bool load_index_from_memory(Index& index, const std::uint8_t* data, std::size_t size);

} // namespace mse

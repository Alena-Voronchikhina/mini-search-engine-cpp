#pragma once

#include <cstdint>
#include <vector>

namespace mse {

// Variable-byte coding for unsigned integers.
void varbyte_encode(std::uint32_t value, std::vector<std::uint8_t> &out);
[[nodiscard]] bool varbyte_decode(const std::uint8_t *&cur, const std::uint8_t *end,
                                  std::uint32_t &value);

// Delta-encode a sorted ascending sequence (first absolute, then gaps).
[[nodiscard]] std::vector<std::uint32_t> delta_encode(const std::vector<std::uint32_t> &sorted);
[[nodiscard]] std::vector<std::uint32_t> delta_decode(const std::vector<std::uint32_t> &deltas);

// Compress a sorted doc-id (or position) list: delta then varbyte.
[[nodiscard]] std::vector<std::uint8_t> compress_sorted(const std::vector<std::uint32_t> &sorted);
[[nodiscard]] bool decompress_sorted(const std::vector<std::uint8_t> &bytes,
                                     std::vector<std::uint32_t> &out);

} // namespace mse

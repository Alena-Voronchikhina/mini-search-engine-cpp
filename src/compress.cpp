#include "mse/compress.hpp"

namespace mse {

void varbyte_encode(std::uint32_t value, std::vector<std::uint8_t>& out) {
    while (value >= 0x80u) {
        out.push_back(static_cast<std::uint8_t>((value & 0x7Fu) | 0x80u));
        value >>= 7;
    }
    out.push_back(static_cast<std::uint8_t>(value));
}

bool varbyte_decode(const std::uint8_t*& cur, const std::uint8_t* end, std::uint32_t& value) {
    value = 0;
    int shift = 0;
    while (cur < end) {
        const std::uint8_t b = *cur++;
        value |= static_cast<std::uint32_t>(b & 0x7Fu) << shift;
        if ((b & 0x80u) == 0)
            return true;
        shift += 7;
        if (shift > 28)
            return false;
    }
    return false;
}

std::vector<std::uint32_t> delta_encode(const std::vector<std::uint32_t>& sorted) {
    std::vector<std::uint32_t> deltas;
    deltas.reserve(sorted.size());
    std::uint32_t prev = 0;
    for (std::size_t i = 0; i < sorted.size(); ++i) {
        const std::uint32_t v = sorted[i];
        deltas.push_back(i == 0 ? v : v - prev);
        prev = v;
    }
    return deltas;
}

std::vector<std::uint32_t> delta_decode(const std::vector<std::uint32_t>& deltas) {
    std::vector<std::uint32_t> out;
    out.reserve(deltas.size());
    std::uint32_t prev = 0;
    for (std::size_t i = 0; i < deltas.size(); ++i) {
        prev = (i == 0) ? deltas[i] : prev + deltas[i];
        out.push_back(prev);
    }
    return out;
}

std::vector<std::uint8_t> compress_sorted(const std::vector<std::uint32_t>& sorted) {
    const auto deltas = delta_encode(sorted);
    std::vector<std::uint8_t> out;
    out.reserve(deltas.size() * 2);
    for (std::uint32_t d : deltas)
        varbyte_encode(d, out);
    return out;
}

bool decompress_sorted(const std::vector<std::uint8_t>& bytes, std::vector<std::uint32_t>& out) {
    out.clear();
    const std::uint8_t* cur = bytes.data();
    const std::uint8_t* end = bytes.data() + bytes.size();
    std::vector<std::uint32_t> deltas;
    while (cur < end) {
        std::uint32_t v = 0;
        if (!varbyte_decode(cur, end, v))
            return false;
        deltas.push_back(v);
    }
    out = delta_decode(deltas);
    return true;
}

} // namespace mse

#include "mse/compress.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("varbyte roundtrip", "[compress]") {
    std::vector<std::uint8_t> bytes;
    for (std::uint32_t v : {0u, 1u, 127u, 128u, 255u, 16384u, 0xFFFFFFFFu}) {
        bytes.clear();
        mse::varbyte_encode(v, bytes);
        const std::uint8_t *cur = bytes.data();
        std::uint32_t out = 0;
        REQUIRE(mse::varbyte_decode(cur, bytes.data() + bytes.size(), out));
        REQUIRE(out == v);
        REQUIRE(cur == bytes.data() + bytes.size());
    }
}

TEST_CASE("delta roundtrip", "[compress]") {
    std::vector<std::uint32_t> ids{0, 1, 5, 100, 101, 1000};
    auto deltas = mse::delta_encode(ids);
    REQUIRE(mse::delta_decode(deltas) == ids);
}

TEST_CASE("compress_sorted roundtrip", "[compress]") {
    std::vector<std::uint32_t> ids{2, 4, 8, 16, 32, 64, 128, 256};
    auto bytes = mse::compress_sorted(ids);
    REQUIRE(bytes.size() < ids.size() * sizeof(std::uint32_t));
    std::vector<std::uint32_t> out;
    REQUIRE(mse::decompress_sorted(bytes, out));
    REQUIRE(out == ids);
}

TEST_CASE("compress empty", "[compress]") {
    auto bytes = mse::compress_sorted({});
    std::vector<std::uint32_t> out;
    REQUIRE(mse::decompress_sorted(bytes, out));
    REQUIRE(out.empty());
}

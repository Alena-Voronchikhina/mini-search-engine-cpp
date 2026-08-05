#include <catch2/catch_test_macros.hpp>

#include "mse/intersect.hpp"

#include <numeric>

TEST_CASE("skip lists match two-pointer intersection", "[skip]") {
    std::vector<mse::DocId> a(500);
    std::iota(a.begin(), a.end(), 0);
    std::vector<mse::DocId> b;
    for (mse::DocId i = 0; i < 500; i += 7)
        b.push_back(i);

    auto tp = mse::intersect_two_pointer(a, b);
    auto sk = mse::intersect_with_skips(a, b);
    auto gal = mse::intersect_galloping(a, b);
    REQUIRE(tp == sk);
    REQUIRE(tp == gal);
    REQUIRE(tp == b);
}

TEST_CASE("skip list empty and singleton", "[skip]") {
    REQUIRE(mse::intersect_with_skips({}, {1, 2}).empty());
    REQUIRE(mse::intersect_with_skips({3}, {3}) == std::vector<mse::DocId>{3});
    REQUIRE(mse::intersect_with_skips({1, 2, 3}, {4, 5}).empty());
}

TEST_CASE("make_skip_list interval", "[skip]") {
    std::vector<mse::DocId> docs(100);
    std::iota(docs.begin(), docs.end(), 0);
    auto sl = mse::make_skip_list(docs);
    REQUIRE(sl.interval >= 1);
    REQUIRE_FALSE(sl.skip_to.empty());
}

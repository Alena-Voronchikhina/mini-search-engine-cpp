#include <catch2/catch_test_macros.hpp>

#include "mse/intersect.hpp"

#include <numeric>

using mse::DocId;

TEST_CASE("two-pointer intersection basic", "[intersect]") {
    std::vector<DocId> a{1, 3, 5, 7, 9};
    std::vector<DocId> b{3, 4, 5, 10};
    REQUIRE(mse::intersect_two_pointer(a, b) == std::vector<DocId>{3, 5});
}

TEST_CASE("galloping matches two-pointer", "[intersect]") {
    std::vector<DocId> long_l(1000);
    std::iota(long_l.begin(), long_l.end(), 0);
    std::vector<DocId> short_l;
    for (DocId i = 0; i < 1000; i += 13)
        short_l.push_back(i);
    auto tp = mse::intersect_two_pointer(short_l, long_l);
    auto gal = mse::intersect_galloping(short_l, long_l);
    REQUIRE(tp == gal);
    REQUIRE(tp == short_l);
}

TEST_CASE("galloping empty and disjoint", "[intersect]") {
    std::vector<DocId> a{1, 2, 3};
    std::vector<DocId> b{4, 5, 6};
    REQUIRE(mse::intersect_galloping(a, b).empty());
    REQUIRE(mse::intersect_galloping({}, a).empty());
}

TEST_CASE("unite and difference", "[intersect]") {
    std::vector<DocId> a{1, 2, 3, 5};
    std::vector<DocId> b{2, 4, 5};
    REQUIRE(mse::unite(a, b) == std::vector<DocId>{1, 2, 3, 4, 5});
    REQUIRE(mse::difference(a, b) == std::vector<DocId>{1, 3});
}

TEST_CASE("posting_docs extracts ids", "[intersect]") {
    std::vector<mse::Posting> p{{1, {0}}, {4, {1, 2}}};
    REQUIRE(mse::posting_docs(p) == std::vector<DocId>{1, 4});
}

#include "mse/stemmer.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("porter stem common forms", "[stemmer]") {
    REQUIRE(mse::porter_stem("caresses") == "caress");
    REQUIRE(mse::porter_stem("ponies") == "poni");
    REQUIRE(mse::porter_stem("cats") == "cat");
    REQUIRE(mse::porter_stem("running") == "run");
    REQUIRE(mse::porter_stem("relational") == "relat");
}

TEST_CASE("porter short words unchanged-ish", "[stemmer]") {
    REQUIRE(mse::porter_stem("a") == "a");
    REQUIRE(mse::porter_stem("be") == "be");
}

TEST_CASE("porter lowercases", "[stemmer]") {
    REQUIRE(mse::porter_stem("Running") == mse::porter_stem("running"));
}

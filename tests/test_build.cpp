#include "mse/build.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

TEST_CASE("parallel build matches serial queries", "[build]") {
    const std::filesystem::path fixtures = "data/fixtures";
    REQUIRE(std::filesystem::exists(fixtures));

    mse::BuildOptions serial_opts;
    serial_opts.threads = 1;
    auto serial = mse::build_index_from_dir(fixtures, serial_opts);

    mse::BuildOptions parallel_opts;
    parallel_opts.threads = 4;
    auto parallel = mse::build_index_from_dir(fixtures, parallel_opts);

    REQUIRE(serial.num_docs() == parallel.num_docs());
    REQUIRE(serial.num_docs() >= 5);

    mse::QueryParser p;
    auto ast = p.parse("cats AND milk");
    REQUIRE(ast);
    auto a = mse::evaluate_boolean(serial, *ast);
    auto b = mse::evaluate_boolean(parallel, *ast);
    REQUIRE(a == b);
}

TEST_CASE("build missing dir yields empty index", "[build]") {
    auto ix = mse::build_index_from_dir("data/does-not-exist-mse");
    REQUIRE(ix.num_docs() == 0);
}

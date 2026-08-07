#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>

namespace {

mse::Index tiny_index() {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("d0", tok.tokenize("cats love milk"));
    ix.add_document("d1", tok.tokenize("dogs and cats play"));
    ix.add_document("d2", tok.tokenize("milk and honey"));
    ix.finalize();
    return ix;
}

std::vector<mse::DocId> q(const mse::Index &ix, const char *query) {
    mse::QueryParser p;
    auto ast = p.parse(query);
    REQUIRE(ast);
    return mse::evaluate_boolean(ix, *ast);
}

} // namespace

TEST_CASE("boolean AND", "[eval]") {
    auto ix = tiny_index();
    auto hits = q(ix, "cats AND milk");
    REQUIRE(hits == std::vector<mse::DocId>{0});
}

TEST_CASE("boolean OR", "[eval]") {
    auto ix = tiny_index();
    auto hits = q(ix, "dogs OR honey");
    REQUIRE(hits == std::vector<mse::DocId>{1, 2});
}

TEST_CASE("boolean NOT", "[eval]") {
    auto ix = tiny_index();
    auto hits = q(ix, "cats AND NOT milk");
    REQUIRE(hits == std::vector<mse::DocId>{1});
}

TEST_CASE("boolean parens", "[eval]") {
    auto ix = tiny_index();
    auto hits = q(ix, "(dogs OR honey) AND milk");
    REQUIRE(hits == std::vector<mse::DocId>{2});
}

TEST_CASE("boolean missing term", "[eval]") {
    auto ix = tiny_index();
    REQUIRE(q(ix, "unicorn").empty());
}

TEST_CASE("two-pointer and galloping agree on AND", "[eval]") {
    auto ix = tiny_index();
    mse::QueryParser p;
    auto ast = p.parse("cats AND play");
    REQUIRE(ast);
    auto a = mse::evaluate_boolean(ix, *ast, mse::IntersectMode::TwoPointer);
    auto b = mse::evaluate_boolean(ix, *ast, mse::IntersectMode::Galloping);
    REQUIRE(a == b);
}

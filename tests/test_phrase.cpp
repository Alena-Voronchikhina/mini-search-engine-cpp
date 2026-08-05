#include <catch2/catch_test_macros.hpp>

#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/tokenizer.hpp"

TEST_CASE("phrase requires adjacency", "[phrase]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("cats love milk"));
    ix.add_document("b", tok.tokenize("cats drink milk love"));
    ix.finalize();

    mse::QueryParser p;
    auto ast = p.parse("\"cats love\"");
    REQUIRE(ast);
    auto hits = mse::evaluate_boolean(ix, *ast);
    REQUIRE(hits == std::vector<mse::DocId>{0});
}

TEST_CASE("phrase three terms", "[phrase]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("the quick brown fox"));
    ix.add_document("b", tok.tokenize("quick fox brown"));
    ix.finalize();

    mse::QueryParser p;
    auto ast = p.parse("\"quick brown fox\"");
    REQUIRE(ast);
    auto hits = mse::evaluate_boolean(ix, *ast);
    REQUIRE(hits == std::vector<mse::DocId>{0});
}

TEST_CASE("phrase combined with AND", "[phrase]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("cats love milk"));
    ix.add_document("b", tok.tokenize("cats love dogs"));
    ix.finalize();

    mse::QueryParser p;
    auto ast = p.parse("\"cats love\" AND milk");
    REQUIRE(ast);
    auto hits = mse::evaluate_boolean(ix, *ast);
    REQUIRE(hits == std::vector<mse::DocId>{0});
}

TEST_CASE("phrase_matches helper", "[phrase]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("adjacent token positions"));
    ix.finalize();
    REQUIRE(mse::phrase_matches(ix, {"adjacent", "token"}, 0));
    REQUIRE_FALSE(mse::phrase_matches(ix, {"adjacent", "positions"}, 0));
}

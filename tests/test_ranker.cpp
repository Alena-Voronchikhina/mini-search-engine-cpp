#include <catch2/catch_test_macros.hpp>

#include "mse/index.hpp"
#include "mse/query_parser.hpp"
#include "mse/ranker.hpp"
#include "mse/tokenizer.hpp"

TEST_CASE("bm25 ranks relevant doc higher", "[ranker]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("about-cats", tok.tokenize("cats cats cats love milk"));
    ix.add_document("about-dogs", tok.tokenize("dogs play outside today"));
    ix.add_document("mixed", tok.tokenize("cats and dogs"));
    ix.finalize();

    auto hits = mse::rank_bm25(ix, {"cats"}, 2);
    REQUIRE(hits.size() == 2);
    REQUIRE(hits[0].doc_id == 0);
    REQUIRE(hits[0].score > hits[1].score);
}

TEST_CASE("bm25 respects topk", "[ranker]") {
    mse::Tokenizer tok;
    mse::Index ix;
    for (int i = 0; i < 10; ++i)
        ix.add_document("d" + std::to_string(i), tok.tokenize("shared term unique" + std::to_string(i)));
    ix.finalize();
    auto hits = mse::rank_bm25(ix, {"shared"}, 3);
    REQUIRE(hits.size() == 3);
}

TEST_CASE("bm25 empty query terms", "[ranker]") {
    mse::Index ix;
    ix.add_document("a", {"x"});
    ix.finalize();
    REQUIRE(mse::rank_bm25(ix, {}, 10).empty());
}

TEST_CASE("collect_query_terms walks AST", "[ranker]") {
    mse::QueryParser p;
    auto ast = p.parse("cats AND (milk OR \"brown fox\")");
    REQUIRE(ast);
    auto terms = mse::collect_query_terms(*ast);
    REQUIRE(terms.size() >= 3);
}

TEST_CASE("bm25 with candidate filter", "[ranker]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("cats milk"));
    ix.add_document("b", tok.tokenize("cats dogs"));
    ix.finalize();
    auto hits = mse::rank_bm25(ix, {"cats"}, 10, {1});
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].doc_id == 1);
}

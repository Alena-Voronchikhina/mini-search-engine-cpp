#include <catch2/catch_test_macros.hpp>

#include "mse/tokenizer.hpp"

using mse::Tokenizer;
using mse::TokenizerOptions;

TEST_CASE("tokenizer case folding", "[tokenizer]") {
    Tokenizer t;
    auto toks = t.tokenize("Cats LOVE Milk");
    REQUIRE(toks == std::vector<std::string>{"cats", "love", "milk"});
}

TEST_CASE("tokenizer punctuation becomes separators", "[tokenizer]") {
    Tokenizer t;
    auto toks = t.tokenize("hello, world! foo-bar.");
    REQUIRE(toks == std::vector<std::string>{"hello", "world", "foo", "bar"});
}

TEST_CASE("tokenizer keeps digits", "[tokenizer]") {
    Tokenizer t;
    auto toks = t.tokenize("bm25 score 42");
    REQUIRE(toks == std::vector<std::string>{"bm25", "score", "42"});
}

TEST_CASE("tokenizer stopword removal", "[tokenizer]") {
    TokenizerOptions opts;
    opts.remove_stopwords = true;
    Tokenizer t(opts);
    auto toks = t.tokenize("the cat and the dog");
    REQUIRE(toks == std::vector<std::string>{"cat", "dog"});
}

TEST_CASE("tokenizer optional stemming", "[tokenizer]") {
    TokenizerOptions opts;
    opts.stem = true;
    Tokenizer t(opts);
    auto toks = t.tokenize("running cats");
    REQUIRE(toks.size() == 2);
    REQUIRE(toks[0] == "run");
    REQUIRE(toks[1] == "cat");
}

TEST_CASE("tokenizer empty input", "[tokenizer]") {
    Tokenizer t;
    REQUIRE(t.tokenize("").empty());
    REQUIRE(t.tokenize("   !!!").empty());
}

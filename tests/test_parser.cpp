#include <catch2/catch_test_macros.hpp>

#include "mse/query_parser.hpp"

using mse::NodeKind;
using mse::QueryParser;

TEST_CASE("parse single term", "[parser]") {
    QueryParser p;
    auto ast = p.parse("cats");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::Term);
    REQUIRE(ast->term == "cats");
}

TEST_CASE("parse AND OR NOT precedence", "[parser]") {
    QueryParser p;
    auto ast = p.parse("a OR b AND c");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::Or);
    REQUIRE(ast->right->kind == NodeKind::And);
}

TEST_CASE("parse juxtaposition as AND", "[parser]") {
    QueryParser p;
    auto ast = p.parse("cats milk");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::And);
}

TEST_CASE("parse parentheses", "[parser]") {
    QueryParser p;
    auto ast = p.parse("(a OR b) AND c");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::And);
    REQUIRE(ast->left->kind == NodeKind::Or);
}

TEST_CASE("parse NOT", "[parser]") {
    QueryParser p;
    auto ast = p.parse("NOT cats");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::Not);
    REQUIRE(ast->child->term == "cats");
}

TEST_CASE("parse phrase", "[parser]") {
    QueryParser p;
    auto ast = p.parse("\"cats love\"");
    REQUIRE(ast);
    REQUIRE(ast->kind == NodeKind::Phrase);
    REQUIRE(ast->phrase_terms == std::vector<std::string>{"cats", "love"});
}

TEST_CASE("parser errors", "[parser]") {
    QueryParser p;
    REQUIRE_FALSE(p.parse(""));
    REQUIRE(p.parse("").error().message.find("Empty") != std::string::npos);

    auto uq = p.parse("\"unterminated");
    REQUIRE_FALSE(uq);
    REQUIRE(uq.error().message.find("Unterminated") != std::string::npos);

    auto up = p.parse("(a OR b");
    REQUIRE_FALSE(up);
    REQUIRE(up.error().message.find("Unmatched") != std::string::npos);

    auto ep = p.parse("\"   \"");
    REQUIRE_FALSE(ep);
    REQUIRE(ep.error().message.find("Empty phrase") != std::string::npos);

    auto bad = p.parse("a AND");
    REQUIRE_FALSE(bad);
}

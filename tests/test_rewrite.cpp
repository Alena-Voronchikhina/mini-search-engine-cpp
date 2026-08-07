#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/rewrite.hpp"
#include "mse/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

TEST_CASE("synonym rewrite expands OR", "[rewrite]") {
    mse::SynonymMap syns{{"car", {"car", "automobile"}}, {"automobile", {"car", "automobile"}}};
    mse::QueryParser p;
    auto ast = p.parse("car");
    REQUIRE(ast);
    auto rewritten = mse::rewrite_synonyms(*ast, syns);
    REQUIRE(rewritten);
    REQUIRE(rewritten->kind == mse::NodeKind::Or);
}

TEST_CASE("load synonyms and end-to-end", "[rewrite]") {
    const auto path = (std::filesystem::temp_directory_path() / "mse_synonyms.txt").string();
    {
        std::ofstream out(path);
        out << "cat,kitten,feline\n";
        out << "# comment\n";
        out << "dog,puppy\n";
    }
    auto map = mse::load_synonyms(path);
    REQUIRE(map.count("cat"));
    REQUIRE(map["cat"].size() >= 2);

    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("a", tok.tokenize("a cute kitten plays"));
    ix.add_document("b", tok.tokenize("dogs run"));
    ix.finalize();

    mse::QueryParser p;
    auto ast = p.parse("cat");
    REQUIRE(ast);
    auto rw = mse::rewrite_synonyms(*ast, map);
    auto hits = mse::evaluate_boolean(ix, *rw);
    REQUIRE(hits == std::vector<mse::DocId>{0});

    std::filesystem::remove(path);
}

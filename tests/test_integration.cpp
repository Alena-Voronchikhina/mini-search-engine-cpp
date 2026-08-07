#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/ranker.hpp"
#include "mse/serialize.hpp"
#include "mse/tokenizer.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

using Catch::Approx;

TEST_CASE("end-to-end index fixtures and query", "[integration]") {
    mse::Tokenizer tok;
    mse::Index ix;
    const std::filesystem::path fixtures = "data/fixtures";
    REQUIRE(std::filesystem::exists(fixtures));
    for (auto const &e : std::filesystem::directory_iterator(fixtures)) {
        if (!e.is_regular_file())
            continue;
        std::ifstream in(e.path());
        std::string text((std::istreambuf_iterator<char>(in)), {});
        ix.add_document(e.path().string(), tok.tokenize(text));
    }
    ix.finalize();
    REQUIRE(ix.num_docs() >= 5);

    mse::QueryParser p;
    auto ast = p.parse("cats AND milk");
    REQUIRE(ast);
    auto hits = mse::evaluate_boolean(ix, *ast);
    REQUIRE_FALSE(hits.empty());

    auto phrase = p.parse("\"machine learning\"");
    REQUIRE(phrase);
    auto ph = mse::evaluate_boolean(ix, *phrase);
    REQUIRE(ph.size() == 1);

    auto ranked = mse::rank_bm25(ix, {"search", "bm25"}, 3);
    REQUIRE_FALSE(ranked.empty());
}

TEST_CASE("malformed queries produce helpful errors", "[integration]") {
    mse::QueryParser p;
    auto e1 = p.parse("(cats AND");
    REQUIRE_FALSE(e1);
    REQUIRE_FALSE(e1.error().message.empty());

    auto e2 = p.parse("\"no end");
    REQUIRE_FALSE(e2);
    REQUIRE(e2.error().offset == 0);
}

TEST_CASE("regression: empty corpus queries safe", "[integration][regression]") {
    mse::Index ix;
    ix.finalize();
    mse::QueryParser p;
    auto ast = p.parse("cats");
    REQUIRE(ast);
    REQUIRE(mse::evaluate_boolean(ix, *ast).empty());
    REQUIRE(mse::rank_bm25(ix, {"cats"}, 5).empty());
}

TEST_CASE("regression: serialize empty index", "[integration][regression]") {
    mse::Index ix;
    ix.finalize();
    const auto path = (std::filesystem::temp_directory_path() / "mse_empty.bin").string();
    REQUIRE(mse::save_index(ix, path));
    mse::Index loaded;
    REQUIRE(mse::load_index(loaded, path));
    REQUIRE(loaded.num_docs() == 0);
    std::filesystem::remove(path);
}

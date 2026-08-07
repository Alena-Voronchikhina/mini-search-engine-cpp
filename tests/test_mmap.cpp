#include "mse/index.hpp"
#include "mse/mmap_load.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/serialize.hpp"
#include "mse/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

TEST_CASE("mmap load matches buffered load", "[mmap]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("d0", tok.tokenize("cats love milk"));
    ix.add_document("d1", tok.tokenize("dogs play"));
    ix.finalize();

    const auto path = (std::filesystem::temp_directory_path() / "mse_mmap.bin").string();
    REQUIRE(mse::save_index(ix, path));

    mse::Index a, b;
    REQUIRE(mse::load_index(a, path));
    REQUIRE(mse::load_index_mmap(b, path));
    REQUIRE(a.num_docs() == b.num_docs());

    mse::QueryParser p;
    auto ast = p.parse("cats AND milk");
    REQUIRE(ast);
    REQUIRE(mse::evaluate_boolean(a, *ast) == mse::evaluate_boolean(b, *ast));

    std::filesystem::remove(path);
}

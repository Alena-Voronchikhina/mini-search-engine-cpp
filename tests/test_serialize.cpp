#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/serialize.hpp"
#include "mse/tokenizer.hpp"

#include <filesystem>
#include <fstream>

using Catch::Approx;

TEST_CASE("save and load roundtrip", "[serialize]") {
    mse::Tokenizer tok;
    mse::Index ix;
    ix.add_document("d0", tok.tokenize("cats love milk"));
    ix.add_document("d1", tok.tokenize("dogs play"));
    ix.finalize();

    const auto path = (std::filesystem::temp_directory_path() / "mse_test_index.bin").string();
    REQUIRE(mse::save_index(ix, path));

    mse::Index loaded;
    REQUIRE(mse::load_index(loaded, path));
    REQUIRE(loaded.num_docs() == ix.num_docs());
    REQUIRE(loaded.avgdl() == Approx(ix.avgdl()));
    REQUIRE(loaded.documents()[0].path == "d0");

    mse::QueryParser p;
    auto ast = p.parse("cats AND milk");
    REQUIRE(ast);
    auto a = mse::evaluate_boolean(ix, *ast);
    auto b = mse::evaluate_boolean(loaded, *ast);
    REQUIRE(a == b);

    std::filesystem::remove(path);
}

TEST_CASE("load rejects bad magic", "[serialize]") {
    const auto path = (std::filesystem::temp_directory_path() / "mse_bad.bin").string();
    {
        std::ofstream out(path, std::ios::binary);
        out << "XXXX";
    }
    mse::Index ix;
    REQUIRE_FALSE(mse::load_index(ix, path));
    std::filesystem::remove(path);
}

TEST_CASE("load missing file fails", "[serialize]") {
    mse::Index ix;
    REQUIRE_FALSE(mse::load_index(ix, "/tmp/mse_does_not_exist_12345.bin"));
}

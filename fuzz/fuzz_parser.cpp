// Fuzz target for tokenizer + query parser.
// Build with MSE_BUILD_FUZZ=ON. On Linux+Clang with MSE_FUZZER_LIBFUZZER=ON,
// links -fsanitize=fuzzer. Otherwise builds a standalone smoke runner.
#include "mse/query_parser.hpp"
#include "mse/tokenizer.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > 4096)
        return 0;
    std::string_view sv(reinterpret_cast<const char *>(data), size);

    mse::TokenizerOptions opts;
    opts.stem = (data[0] & 1) != 0;
    opts.remove_stopwords = (data[0] & 2) != 0;

    mse::Tokenizer tok(opts);
    (void)tok.tokenize(sv);

    mse::QueryParser parser(opts);
    (void)parser.parse(sv);
    return 0;
}

#if defined(MSE_FUZZER_STANDALONE)
int main() {
    const std::vector<std::string> samples = {
        "cats AND milk",        "\"unterminated",
        "(a OR b) AND NOT c",   "(((((",
        "\"quick brown fox\"",  "",
        std::string(3000, 'a'), std::string("\xff OR AND \"(( NOT", 16),
    };
    for (const auto &s : samples) {
        LLVMFuzzerTestOneInput(reinterpret_cast<const uint8_t *>(s.data()), s.size());
    }
    return 0;
}
#endif

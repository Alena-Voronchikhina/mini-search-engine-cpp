#pragma once

#include "mse/types.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace mse {

class Tokenizer {
public:
    explicit Tokenizer(TokenizerOptions opts = {});

    [[nodiscard]] std::vector<std::string> tokenize(std::string_view text) const;

private:
    TokenizerOptions opts_;
};

} // namespace mse

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mse {

using DocId = std::uint32_t;
using TermId = std::uint32_t;
using Pos = std::uint32_t;

struct Posting {
    DocId doc_id{};
    std::vector<Pos> positions;
};

struct RankedHit {
    DocId doc_id{};
    double score{};
};

struct DocumentMeta {
    std::string path;
    std::uint32_t length{}; // token count
};

struct TokenizerOptions {
    bool remove_stopwords{false};
    bool stem{false};
};

struct QueryError {
    std::size_t offset{};
    std::string message;
};

} // namespace mse

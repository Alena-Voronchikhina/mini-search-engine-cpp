#include "mse/tokenizer.hpp"

#include "mse/stemmer.hpp"

#include <cctype>
#include <unordered_set>

namespace mse {
namespace {

const std::unordered_set<std::string>& stopwords() {
    static const std::unordered_set<std::string> k = {
        "a",    "an",  "and",  "are", "as",   "at",   "be",   "by",   "for",  "from",
        "has",  "he",  "in",   "is",  "it",   "its",  "of",   "on",   "that", "the",
        "to",   "was", "were", "will","with", "or",   "not",  "this", "but",  "they",
    };
    return k;
}

} // namespace

Tokenizer::Tokenizer(TokenizerOptions opts) : opts_(opts) {}

std::vector<std::string> Tokenizer::tokenize(std::string_view text) const {
    std::vector<std::string> out;
    std::string cur;
    auto flush = [&] {
        if (cur.empty())
            return;
        if (opts_.remove_stopwords && stopwords().count(cur)) {
            cur.clear();
            return;
        }
        if (opts_.stem)
            cur = porter_stem(cur);
        if (!cur.empty())
            out.push_back(std::move(cur));
        cur.clear();
    };

    for (char ch : text) {
        const unsigned char uc = static_cast<unsigned char>(ch);
        if (std::isalpha(uc)) {
            cur.push_back(static_cast<char>(std::tolower(uc)));
        } else if (std::isdigit(uc)) {
            cur.push_back(static_cast<char>(uc));
        } else {
            flush();
        }
    }
    flush();
    return out;
}

} // namespace mse

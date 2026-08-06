#include "mse/query_parser.hpp"

#include "mse/tokenizer.hpp"

#include <cctype>
#include <memory>
#include <string>
#include <vector>

namespace mse {
namespace {

enum class TokKind { Term, Phrase, And, Or, Not, LParen, RParen, End };

struct LexTok {
    TokKind kind{TokKind::End};
    std::string text;
    std::size_t offset{};
    std::vector<std::string> phrase_terms;
};

class Lexer {
  public:
    Lexer(std::string_view in, TokenizerOptions opts) : in_(in), tok_(opts) {}

    LexTok next() {
        skip_ws();
        if (pos_ >= in_.size())
            return {TokKind::End, {}, pos_, {}};

        const std::size_t start = pos_;
        char c = in_[pos_];
        if (c == '(') {
            ++pos_;
            return {TokKind::LParen, "(", start, {}};
        }
        if (c == ')') {
            ++pos_;
            return {TokKind::RParen, ")", start, {}};
        }
        if (c == '"') {
            ++pos_;
            std::string body;
            while (pos_ < in_.size() && in_[pos_] != '"')
                body.push_back(in_[pos_++]);
            if (pos_ >= in_.size())
                return {TokKind::End, {}, start, {}};
            ++pos_;
            auto terms = tok_.tokenize(body);
            return {TokKind::Phrase, body, start, std::move(terms)};
        }

        std::string word;
        while (pos_ < in_.size() && !std::isspace(static_cast<unsigned char>(in_[pos_])) &&
               in_[pos_] != '(' && in_[pos_] != ')' && in_[pos_] != '"') {
            word.push_back(in_[pos_++]);
        }
        std::string upper = word;
        for (char &ch : upper)
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (upper == "AND")
            return {TokKind::And, word, start, {}};
        if (upper == "OR")
            return {TokKind::Or, word, start, {}};
        if (upper == "NOT")
            return {TokKind::Not, word, start, {}};

        auto terms = tok_.tokenize(word);
        std::string norm = terms.empty() ? std::string{} : terms.front();
        if (norm.empty()) {
            for (char ch : word) {
                if (std::isalnum(static_cast<unsigned char>(ch)))
                    norm.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
        }
        return {TokKind::Term, norm, start, {}};
    }

  private:
    void skip_ws() {
        while (pos_ < in_.size() && std::isspace(static_cast<unsigned char>(in_[pos_])))
            ++pos_;
    }

    std::string_view in_;
    Tokenizer tok_;
    std::size_t pos_{0};
};

class Parser {
  public:
    Parser(std::string_view in, TokenizerOptions opts) : lex_(in, opts) { advance(); }

    ParseResult parse() {
        auto root = parse_or();
        if (!root)
            return root;
        if (cur_.kind != TokKind::End)
            return ParseResult::fail(QueryError{cur_.offset, "Unexpected token after query"});
        return root;
    }

  private:
    void advance() { cur_ = lex_.next(); }

    ParseResult parse_or() {
        auto left = parse_and();
        if (!left)
            return left;
        while (cur_.kind == TokKind::Or) {
            advance();
            auto right = parse_and();
            if (!right)
                return right;
            auto n = std::make_unique<QueryNode>();
            n->kind = NodeKind::Or;
            n->left = std::move(left.node());
            n->right = std::move(right.node());
            left = ParseResult::ok(std::move(n));
        }
        return left;
    }

    ParseResult parse_and() {
        auto left = parse_unary();
        if (!left)
            return left;
        while (true) {
            if (cur_.kind == TokKind::And) {
                advance();
                auto right = parse_unary();
                if (!right)
                    return right;
                auto n = std::make_unique<QueryNode>();
                n->kind = NodeKind::And;
                n->left = std::move(left.node());
                n->right = std::move(right.node());
                left = ParseResult::ok(std::move(n));
                continue;
            }
            if (cur_.kind == TokKind::Term || cur_.kind == TokKind::Phrase ||
                cur_.kind == TokKind::LParen || cur_.kind == TokKind::Not) {
                auto right = parse_unary();
                if (!right)
                    return right;
                auto n = std::make_unique<QueryNode>();
                n->kind = NodeKind::And;
                n->left = std::move(left.node());
                n->right = std::move(right.node());
                left = ParseResult::ok(std::move(n));
                continue;
            }
            break;
        }
        return left;
    }

    ParseResult parse_unary() {
        if (cur_.kind == TokKind::Not) {
            advance();
            auto child = parse_unary();
            if (!child)
                return child;
            auto n = std::make_unique<QueryNode>();
            n->kind = NodeKind::Not;
            n->child = std::move(child.node());
            return ParseResult::ok(std::move(n));
        }
        return parse_primary();
    }

    ParseResult parse_primary() {
        if (cur_.kind == TokKind::LParen) {
            const std::size_t open_off = cur_.offset;
            advance();
            auto inner = parse_or();
            if (!inner)
                return inner;
            if (cur_.kind != TokKind::RParen)
                return ParseResult::fail(QueryError{open_off, "Unmatched '(' — expected ')'"});
            advance();
            return inner;
        }
        if (cur_.kind == TokKind::Term) {
            if (cur_.text.empty())
                return ParseResult::fail(QueryError{cur_.offset, "Empty term after tokenization"});
            auto n = std::make_unique<QueryNode>();
            n->kind = NodeKind::Term;
            n->term = cur_.text;
            advance();
            return ParseResult::ok(std::move(n));
        }
        if (cur_.kind == TokKind::Phrase) {
            auto n = std::make_unique<QueryNode>();
            n->kind = NodeKind::Phrase;
            n->phrase_terms = cur_.phrase_terms;
            if (n->phrase_terms.empty())
                return ParseResult::fail(QueryError{cur_.offset, "Empty phrase query"});
            advance();
            return ParseResult::ok(std::move(n));
        }
        if (cur_.kind == TokKind::End)
            return ParseResult::fail(QueryError{cur_.offset, "Unexpected end of query"});
        if (cur_.kind == TokKind::RParen)
            return ParseResult::fail(QueryError{cur_.offset, "Unexpected ')'"});
        return ParseResult::fail(QueryError{cur_.offset, "Unexpected token"});
    }

    Lexer lex_;
    LexTok cur_{};
};

} // namespace

QueryParser::QueryParser(TokenizerOptions tok_opts) : tok_opts_(tok_opts) {}

ParseResult QueryParser::parse(std::string_view input) const {
    bool in_quote = false;
    std::size_t quote_off = 0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '"') {
            in_quote = !in_quote;
            if (in_quote)
                quote_off = i;
        }
    }
    if (in_quote)
        return ParseResult::fail(QueryError{quote_off, "Unterminated phrase (missing '\"')"});

    std::string_view trimmed = input;
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front())))
        trimmed.remove_prefix(1);
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())))
        trimmed.remove_suffix(1);
    if (trimmed.empty())
        return ParseResult::fail(QueryError{0, "Empty query"});

    Parser p(trimmed, tok_opts_);
    return p.parse();
}

} // namespace mse

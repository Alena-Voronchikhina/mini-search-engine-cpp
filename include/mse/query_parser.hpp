#pragma once

#include "mse/query_ast.hpp"
#include "mse/tokenizer.hpp"
#include "mse/types.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace mse {

class ParseResult {
public:
    static ParseResult ok(std::unique_ptr<QueryNode> node) {
        ParseResult r;
        r.data_ = std::move(node);
        return r;
    }
    static ParseResult fail(QueryError err) {
        ParseResult r;
        r.data_ = std::move(err);
        return r;
    }

    [[nodiscard]] explicit operator bool() const {
        return std::holds_alternative<std::unique_ptr<QueryNode>>(data_);
    }
    [[nodiscard]] QueryNode& operator*() { return *std::get<std::unique_ptr<QueryNode>>(data_); }
    [[nodiscard]] const QueryNode& operator*() const {
        return *std::get<std::unique_ptr<QueryNode>>(data_);
    }
    [[nodiscard]] QueryNode* operator->() { return std::get<std::unique_ptr<QueryNode>>(data_).get(); }
    [[nodiscard]] const QueryNode* operator->() const {
        return std::get<std::unique_ptr<QueryNode>>(data_).get();
    }
    [[nodiscard]] const QueryError& error() const { return std::get<QueryError>(data_); }
    [[nodiscard]] std::unique_ptr<QueryNode>& node() {
        return std::get<std::unique_ptr<QueryNode>>(data_);
    }

private:
    std::variant<std::unique_ptr<QueryNode>, QueryError> data_{QueryError{}};
};

class QueryParser {
public:
    explicit QueryParser(TokenizerOptions tok_opts = {});

    [[nodiscard]] ParseResult parse(std::string_view input) const;

private:
    TokenizerOptions tok_opts_;
};

} // namespace mse

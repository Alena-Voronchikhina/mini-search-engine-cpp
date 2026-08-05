#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace mse {

enum class NodeKind { Term, Phrase, And, Or, Not };

struct QueryNode {
    NodeKind kind{NodeKind::Term};
    std::string term;                       // Term
    std::vector<std::string> phrase_terms;  // Phrase
    std::unique_ptr<QueryNode> child;       // Not
    std::unique_ptr<QueryNode> left;        // And/Or
    std::unique_ptr<QueryNode> right;       // And/Or
};

} // namespace mse

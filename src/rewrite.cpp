#include "mse/rewrite.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace mse {
namespace {

std::string lower(std::string s) {
    for (char &c : s)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::unique_ptr<QueryNode> or_chain(std::vector<std::unique_ptr<QueryNode>> nodes) {
    if (nodes.empty())
        return nullptr;
    std::unique_ptr<QueryNode> acc = std::move(nodes[0]);
    for (std::size_t i = 1; i < nodes.size(); ++i) {
        auto n = std::make_unique<QueryNode>();
        n->kind = NodeKind::Or;
        n->left = std::move(acc);
        n->right = std::move(nodes[i]);
        acc = std::move(n);
    }
    return acc;
}

std::unique_ptr<QueryNode> clone_rewrite(const QueryNode &n, const SynonymMap &syns);

std::unique_ptr<QueryNode> expand_term(const std::string &term, const SynonymMap &syns) {
    std::vector<std::unique_ptr<QueryNode>> alts;
    auto self = std::make_unique<QueryNode>();
    self->kind = NodeKind::Term;
    self->term = term;
    alts.push_back(std::move(self));

    auto it = syns.find(term);
    if (it != syns.end()) {
        for (const auto &alt : it->second) {
            if (alt == term)
                continue;
            auto t = std::make_unique<QueryNode>();
            t->kind = NodeKind::Term;
            t->term = alt;
            alts.push_back(std::move(t));
        }
    }
    return or_chain(std::move(alts));
}

std::unique_ptr<QueryNode> clone_rewrite(const QueryNode &n, const SynonymMap &syns) {
    switch (n.kind) {
    case NodeKind::Term:
        return expand_term(n.term, syns);
    case NodeKind::Phrase: {
        auto out = std::make_unique<QueryNode>();
        out->kind = NodeKind::Phrase;
        out->phrase_terms = n.phrase_terms;
        return out;
    }
    case NodeKind::Not: {
        auto out = std::make_unique<QueryNode>();
        out->kind = NodeKind::Not;
        out->child = clone_rewrite(*n.child, syns);
        return out;
    }
    case NodeKind::And:
    case NodeKind::Or: {
        auto out = std::make_unique<QueryNode>();
        out->kind = n.kind;
        out->left = clone_rewrite(*n.left, syns);
        out->right = clone_rewrite(*n.right, syns);
        return out;
    }
    }
    return nullptr;
}

} // namespace

SynonymMap load_synonyms(const std::string &path) {
    SynonymMap map;
    std::ifstream in(path);
    if (!in)
        return map;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            // trim
            while (!cell.empty() && std::isspace(static_cast<unsigned char>(cell.front())))
                cell.erase(cell.begin());
            while (!cell.empty() && std::isspace(static_cast<unsigned char>(cell.back())))
                cell.pop_back();
            if (!cell.empty())
                parts.push_back(lower(cell));
        }
        if (parts.size() < 2)
            continue;
        for (const auto &key : parts) {
            auto &bucket = map[key];
            for (const auto &v : parts) {
                if (std::find(bucket.begin(), bucket.end(), v) == bucket.end())
                    bucket.push_back(v);
            }
        }
    }
    return map;
}

std::unique_ptr<QueryNode> rewrite_synonyms(const QueryNode &root, const SynonymMap &syns) {
    if (syns.empty()) {
        // deep clone via expand with empty map = identity for terms
        return clone_rewrite(root, syns);
    }
    return clone_rewrite(root, syns);
}

} // namespace mse

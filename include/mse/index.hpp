#pragma once

#include "mse/types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace mse {

class Index {
public:
    void add_document(std::string path, const std::vector<std::string>& tokens);
    void finalize();

    [[nodiscard]] const std::vector<DocumentMeta>& documents() const { return docs_; }
    [[nodiscard]] std::size_t num_docs() const { return docs_.size(); }
    [[nodiscard]] double avgdl() const { return avgdl_; }

    [[nodiscard]] const std::vector<Posting>* postings(const std::string& term) const;
    [[nodiscard]] std::uint32_t df(const std::string& term) const;
    [[nodiscard]] std::uint32_t tf(const std::string& term, DocId doc) const;

    [[nodiscard]] std::vector<DocId> all_doc_ids() const;

    // Friends for serialization.
    friend bool save_index(const Index& index, const std::string& path);
    friend bool load_index(Index& index, const std::string& path);

private:
    std::vector<DocumentMeta> docs_;
    std::unordered_map<std::string, std::vector<Posting>> inv_;
    double avgdl_{0.0};
    bool finalized_{false};
};

} // namespace mse

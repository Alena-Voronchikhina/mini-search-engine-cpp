#include "mse/build.hpp"

#include <algorithm>
#include <fstream>
#include <thread>

namespace mse {
namespace {

bool read_file(const std::filesystem::path& p, std::string& out) {
    std::ifstream in(p);
    if (!in)
        return false;
    out.assign(std::istreambuf_iterator<char>(in), {});
    return true;
}

std::vector<std::filesystem::path> collect_docs(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> paths;
    if (!std::filesystem::exists(dir))
        return paths;
    for (auto const& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (!entry.is_regular_file())
            continue;
        const auto ext = entry.path().extension().string();
        if (ext == ".txt" || ext == ".md")
            paths.push_back(entry.path());
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

} // namespace

Index build_index_from_dir(const std::filesystem::path& dir, BuildOptions opts) {
    auto paths = collect_docs(dir);
    std::size_t nthreads = opts.threads;
    if (nthreads == 0)
        nthreads = std::max<std::size_t>(1, std::thread::hardware_concurrency());
    nthreads = std::max<std::size_t>(1, nthreads);

    struct Item {
        std::string path;
        std::vector<std::string> tokens;
    };
    std::vector<Item> corpus(paths.size());

    if (nthreads == 1 || paths.size() < 2) {
        Tokenizer tok(opts.tokenizer);
        for (std::size_t i = 0; i < paths.size(); ++i) {
            std::string text;
            if (!read_file(paths[i], text))
                continue;
            corpus[i] = Item{paths[i].string(), tok.tokenize(text)};
        }
    } else {
        nthreads = std::min(nthreads, paths.size());
        std::vector<std::thread> workers;
        workers.reserve(nthreads);
        for (std::size_t t = 0; t < nthreads; ++t) {
            workers.emplace_back([&, t] {
                Tokenizer local_tok(opts.tokenizer);
                for (std::size_t i = t; i < paths.size(); i += nthreads) {
                    std::string text;
                    if (!read_file(paths[i], text))
                        continue;
                    corpus[i] = Item{paths[i].string(), local_tok.tokenize(text)};
                }
            });
        }
        for (auto& w : workers)
            w.join();
    }

    Index index;
    for (auto& it : corpus) {
        if (it.path.empty())
            continue;
        index.add_document(std::move(it.path), it.tokens);
    }
    index.finalize();
    return index;
}

} // namespace mse

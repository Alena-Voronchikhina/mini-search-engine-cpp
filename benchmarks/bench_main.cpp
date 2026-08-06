#include "mse/index.hpp"
#include "mse/intersect.hpp"
#include "mse/query_parser.hpp"
#include "mse/ranker.hpp"
#include "mse/rss.hpp"
#include "mse/serialize.hpp"
#include "mse/tokenizer.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static double percentile(std::vector<double> v, double p) {
    if (v.empty())
        return 0;
    std::sort(v.begin(), v.end());
    const double idx = p * static_cast<double>(v.size() - 1);
    const auto i = static_cast<std::size_t>(idx);
    const double frac = idx - static_cast<double>(i);
    if (i + 1 >= v.size())
        return v.back();
    return v[i] * (1.0 - frac) + v[i + 1] * frac;
}

int main(int argc, char *argv[]) {
    std::size_t ndocs = 5000;
    std::string out_path;
    std::uint32_t seed = 42;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--docs" && i + 1 < argc)
            ndocs = static_cast<std::size_t>(std::stoul(argv[++i]));
        else if (a == "--out" && i + 1 < argc)
            out_path = argv[++i];
        else if (a == "--seed" && i + 1 < argc)
            seed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
    }

    static const char *vocab[] = {
        "cat",       "dog",    "milk",    "love",      "play",    "search", "engine",
        "index",     "query",  "rank",    "token",     "stem",    "score",  "document",
        "posting",   "phrase", "boolean", "vector",    "latency", "memory", "disk",
        "benchmark", "gallop", "pointer", "intersect", "bm25",
    };
    constexpr std::size_t vocab_n = sizeof(vocab) / sizeof(vocab[0]);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> len_dist(20, 80);
    std::uniform_int_distribution<std::size_t> term_dist(0, vocab_n - 1);

    mse::Tokenizer tok;
    mse::Index index;
    const auto t0 = std::chrono::steady_clock::now();
    for (std::size_t d = 0; d < ndocs; ++d) {
        std::ostringstream oss;
        const int L = len_dist(rng);
        for (int i = 0; i < L; ++i) {
            if (i)
                oss << ' ';
            oss << vocab[term_dist(rng)];
        }
        index.add_document("doc_" + std::to_string(d) + ".txt", tok.tokenize(oss.str()));
    }
    index.finalize();
    const auto t1 = std::chrono::steady_clock::now();
    const double build_s = std::chrono::duration<double>(t1 - t0).count();

    const fs::path tmp_idx = fs::temp_directory_path() / "mse_bench_index.bin";
    if (!mse::save_index(index, tmp_idx.string()))
        return 1;
    const auto idx_size = fs::file_size(tmp_idx);

    // Skewed lengths: galloping wins when probing a long list from a sparse short list.
    const std::size_t long_n = std::max(ndocs * 10, std::size_t{50000});
    std::vector<mse::DocId> long_list(long_n);
    std::iota(long_list.begin(), long_list.end(), 0);
    std::vector<mse::DocId> short_list;
    for (mse::DocId i = 0; i < static_cast<mse::DocId>(long_n); i += 997)
        short_list.push_back(i);

    auto time_fn = [&](auto fn) {
        std::vector<double> samples;
        for (int i = 0; i < 200; ++i) {
            const auto a = std::chrono::steady_clock::now();
            auto r = fn(short_list, long_list);
            volatile auto sink = r.size();
            (void)sink;
            const auto b = std::chrono::steady_clock::now();
            samples.push_back(std::chrono::duration<double, std::micro>(b - a).count());
        }
        return samples;
    };
    auto two_s = time_fn(mse::intersect_two_pointer);
    auto gal_s = time_fn(mse::intersect_galloping);

    mse::QueryParser parser;
    std::vector<std::string> queries = {"cat dog", "milk AND love", "search OR engine", "bm25"};
    std::vector<double> q_us;
    for (int i = 0; i < 100; ++i) {
        auto ast = parser.parse(queries[static_cast<std::size_t>(i) % queries.size()]);
        if (!ast)
            continue;
        const auto a = std::chrono::steady_clock::now();
        auto terms = mse::collect_query_terms(*ast);
        auto ranked = mse::rank_bm25(index, terms, 10, {});
        (void)ranked;
        const auto b = std::chrono::steady_clock::now();
        q_us.push_back(std::chrono::duration<double, std::micro>(b - a).count());
    }

    const double p95_two = percentile(two_s, 0.95);
    const double p95_gal = percentile(gal_s, 0.95);
    const double improvement = p95_two > 0 ? (1.0 - p95_gal / p95_two) * 100.0 : 0.0;
    const auto rss = mse::rss_bytes();

    std::ostringstream report;
    report << "# Benchmark results\n\n";
    report << "- Docs: " << ndocs << " (seed=" << seed << ")\n";
    report << "- Build time: " << build_s << " s (" << (ndocs / std::max(build_s, 1e-9))
           << " docs/s)\n";
    report << "- Index size on disk: " << idx_size << " bytes\n";
    report << "- RSS after build: " << rss << " bytes\n";
    report << "- BM25 query latency: p50=" << percentile(q_us, 0.50)
           << " us, p95=" << percentile(q_us, 0.95) << " us\n";
    report << "- Intersect two-pointer: p50=" << percentile(two_s, 0.50) << " us, p95=" << p95_two
           << " us\n";
    report << "- Intersect galloping: p50=" << percentile(gal_s, 0.50) << " us, p95=" << p95_gal
           << " us\n";
    report << "- Galloping vs two-pointer p95 improvement: " << improvement << "%\n";
    std::cout << report.str();
    if (!out_path.empty()) {
        std::ofstream out(out_path);
        out << report.str();
    }
    fs::remove(tmp_idx);
    return 0;
}

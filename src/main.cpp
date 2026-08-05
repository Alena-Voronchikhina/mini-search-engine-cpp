#include "mse/build.hpp"
#include "mse/index.hpp"
#include "mse/query_eval.hpp"
#include "mse/query_parser.hpp"
#include "mse/ranker.hpp"
#include "mse/serialize.hpp"
#include "mse/tokenizer.hpp"
#include "mse/intersect.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

void usage() {
    std::cerr
        << "Usage:\n"
        << "  search index build <dir> -o <index.bin> [--stem] [--stopwords] [--threads N]\n"
        << "  search query <index.bin> <query> [--mode boolean|bm25] [--topk N]\n"
        << "         [--stem] [--stopwords] [--intersect two|gallop]\n"
        << "  search bench [--docs N] [--out path] [--seed S]\n";
}

std::uint64_t rss_bytes() {
#if defined(__APPLE__)
    mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info),
                  &count) != KERN_SUCCESS)
        return 0;
    return static_cast<std::uint64_t>(info.resident_size);
#elif defined(__linux__)
    std::ifstream f("/proc/self/status");
    std::string key;
    while (f >> key) {
        if (key == "VmRSS:") {
            std::uint64_t kb = 0;
            f >> kb;
            return kb * 1024;
        }
        std::string rest;
        std::getline(f, rest);
    }
    return 0;
#else
    return 0;
#endif
}

int cmd_index(std::vector<std::string_view> args) {
    if (args.size() < 2 || args[0] != "build") {
        usage();
        return 2;
    }
    fs::path dir{std::string(args[1])};
    fs::path out_path;
    mse::BuildOptions opts;
    opts.threads = 0; // hardware_concurrency
    for (std::size_t i = 2; i < args.size(); ++i) {
        if (args[i] == "-o" && i + 1 < args.size()) {
            out_path = std::string(args[++i]);
        } else if (args[i] == "--stem") {
            opts.tokenizer.stem = true;
        } else if (args[i] == "--stopwords") {
            opts.tokenizer.remove_stopwords = true;
        } else if (args[i] == "--threads" && i + 1 < args.size()) {
            opts.threads = static_cast<std::size_t>(std::stoul(std::string(args[++i])));
        } else {
            std::cerr << "Unknown arg: " << args[i] << "\n";
            return 2;
        }
    }
    if (out_path.empty()) {
        std::cerr << "Missing -o <index.bin>\n";
        return 2;
    }

    auto index = mse::build_index_from_dir(dir, opts);
    if (!mse::save_index(index, out_path.string())) {
        std::cerr << "Failed to write " << out_path << "\n";
        return 1;
    }
    std::cout << "Indexed " << index.num_docs() << " docs -> " << out_path << "\n";
    return 0;
}

int cmd_query(std::vector<std::string_view> args) {
    if (args.size() < 2) {
        usage();
        return 2;
    }
    const std::string index_path{args[0]};
    const std::string query{args[1]};
    std::string mode = "boolean";
    std::size_t topk = 10;
    mse::TokenizerOptions opts;
    mse::IntersectMode imode = mse::IntersectMode::Galloping;
    for (std::size_t i = 2; i < args.size(); ++i) {
        if (args[i] == "--mode" && i + 1 < args.size()) {
            mode = std::string(args[++i]);
        } else if (args[i] == "--topk" && i + 1 < args.size()) {
            topk = static_cast<std::size_t>(std::stoul(std::string(args[++i])));
        } else if (args[i] == "--stem") {
            opts.stem = true;
        } else if (args[i] == "--stopwords") {
            opts.remove_stopwords = true;
        } else if (args[i] == "--intersect" && i + 1 < args.size()) {
            auto v = args[++i];
            imode = (v == "two") ? mse::IntersectMode::TwoPointer : mse::IntersectMode::Galloping;
        } else {
            std::cerr << "Unknown arg: " << args[i] << "\n";
            return 2;
        }
    }

    mse::Index index;
    if (!mse::load_index(index, index_path)) {
        std::cerr << "Failed to load index: " << index_path << "\n";
        return 1;
    }
    mse::QueryParser parser(opts);
    auto ast = parser.parse(query);
    if (!ast) {
        std::cerr << "Query error at " << ast.error().offset << ": " << ast.error().message << "\n";
        return 1;
    }

    if (mode == "boolean") {
        auto hits = mse::evaluate_boolean(index, *ast, imode);
        if (hits.empty()) {
            std::cout << "No results\n";
            return 0;
        }
        for (auto id : hits)
            std::cout << index.documents()[id].path << "\n";
        return 0;
    }

    if (mode == "bm25") {
        auto terms = mse::collect_query_terms(*ast);
        std::vector<mse::DocId> candidates;
        // Operators present → boolean filter then re-rank.
        bool has_ops = false;
        std::function<void(const mse::QueryNode&)> check = [&](const mse::QueryNode& n) {
            if (n.kind == mse::NodeKind::And || n.kind == mse::NodeKind::Or ||
                n.kind == mse::NodeKind::Not || n.kind == mse::NodeKind::Phrase)
                has_ops = true;
            if (n.left)
                check(*n.left);
            if (n.right)
                check(*n.right);
            if (n.child)
                check(*n.child);
        };
        check(*ast);
        // Juxtaposition creates And nodes — treat multi-term as filtered if And/Or/Not/Phrase.
        // Simpler rule from plan: boolean filter when operators; pure term lists → BM25.
        // Juxtaposition is AND which is an operator — for "cats milk" we still want BM25 over
        // union of docs. Plan: "pure term lists go straight to BM25" — detect only Term leaves
        // with ANDs from juxtaposition... Use: if tree is only Terms joined by And, treat as
        // bag-of-terms BM25; otherwise boolean filter.
        std::function<bool(const mse::QueryNode&)> only_and_terms = [&](const mse::QueryNode& n) -> bool {
            if (n.kind == mse::NodeKind::Term)
                return true;
            if (n.kind == mse::NodeKind::And)
                return only_and_terms(*n.left) && only_and_terms(*n.right);
            return false;
        };
        if (!only_and_terms(*ast))
            candidates = mse::evaluate_boolean(index, *ast, imode);

        auto ranked = mse::rank_bm25(index, terms, topk, candidates);
        if (ranked.empty()) {
            std::cout << "No results\n";
            return 0;
        }
        for (const auto& h : ranked) {
            std::cout << h.score << "\t" << index.documents()[h.doc_id].path << "\n";
        }
        return 0;
    }

    std::cerr << "Unknown mode: " << mode << "\n";
    return 2;
}

double percentile(std::vector<double> v, double p) {
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

int cmd_bench(std::vector<std::string_view> args) {
    std::size_t ndocs = 5000;
    std::string out_path;
    std::uint32_t seed = 42;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--docs" && i + 1 < args.size())
            ndocs = static_cast<std::size_t>(std::stoul(std::string(args[++i])));
        else if (args[i] == "--out" && i + 1 < args.size())
            out_path = std::string(args[++i]);
        else if (args[i] == "--seed" && i + 1 < args.size())
            seed = static_cast<std::uint32_t>(std::stoul(std::string(args[++i])));
        else {
            std::cerr << "Unknown arg: " << args[i] << "\n";
            return 2;
        }
    }

    static const char* vocab[] = {
        "cat", "dog", "milk", "love", "play", "search", "engine", "index", "query", "rank",
        "token", "stem", "score", "document", "posting", "phrase", "boolean", "vector", "space",
        "latency", "memory", "disk", "benchmark", "gallop", "pointer", "intersect", "bm25",
        "porter", "stopword", "corpus",
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
    if (!mse::save_index(index, tmp_idx.string())) {
        std::cerr << "Failed to save bench index\n";
        return 1;
    }
    const auto idx_size = fs::file_size(tmp_idx);

    // Build synthetic posting lists for intersection microbench.
    const std::size_t long_n = std::max(ndocs * 10, std::size_t{50000});
    std::vector<mse::DocId> long_list(long_n);
    std::iota(long_list.begin(), long_list.end(), 0);
    std::vector<mse::DocId> short_list;
    for (mse::DocId i = 0; i < static_cast<mse::DocId>(long_n); i += 997)
        short_list.push_back(i);

    auto time_intersect = [&](auto fn) {
        std::vector<double> samples;
        samples.reserve(200);
        for (int i = 0; i < 200; ++i) {
            const auto a = std::chrono::steady_clock::now();
            volatile auto sink = fn(short_list, long_list).size();
            (void)sink;
            const auto b = std::chrono::steady_clock::now();
            samples.push_back(std::chrono::duration<double, std::micro>(b - a).count());
        }
        return samples;
    };

    auto two_s = time_intersect(mse::intersect_two_pointer);
    auto gal_s = time_intersect(mse::intersect_galloping);

    // Query latency BM25
    mse::QueryParser parser;
    std::vector<std::string> queries = {"cat dog", "milk AND love", "search OR engine",
                                        "\"index query\"", "bm25 AND latency"};
    std::vector<double> q_us;
    for (int i = 0; i < 100; ++i) {
        const auto& q = queries[static_cast<std::size_t>(i) % queries.size()];
        auto ast = parser.parse(q);
        if (!ast)
            continue;
        const auto a = std::chrono::steady_clock::now();
        auto terms = mse::collect_query_terms(*ast);
        auto ranked = mse::rank_bm25(index, terms, 10, {});
        (void)ranked;
        const auto b = std::chrono::steady_clock::now();
        q_us.push_back(std::chrono::duration<double, std::micro>(b - a).count());
    }

    const auto rss = rss_bytes();
    std::ostringstream report;
    report << "# Benchmark results\n\n";
    report << "- Machine: local bench harness\n";
    report << "- Docs: " << ndocs << " (seed=" << seed << ")\n";
    report << "- Build time: " << build_s << " s (" << (ndocs / build_s) << " docs/s)\n";
    report << "- Index size on disk: " << idx_size << " bytes\n";
    report << "- RSS after build: " << rss << " bytes\n";
    report << "- BM25 query latency: p50=" << percentile(q_us, 0.50)
           << " us, p95=" << percentile(q_us, 0.95) << " us\n";
    report << "- Intersect two-pointer: p50=" << percentile(two_s, 0.50)
           << " us, p95=" << percentile(two_s, 0.95) << " us\n";
    report << "- Intersect galloping: p50=" << percentile(gal_s, 0.50)
           << " us, p95=" << percentile(gal_s, 0.95) << " us\n";
    const double p95_two = percentile(two_s, 0.95);
    const double p95_gal = percentile(gal_s, 0.95);
    const double improvement = p95_two > 0 ? (1.0 - p95_gal / p95_two) * 100.0 : 0.0;
    report << "- Galloping vs two-pointer p95 improvement: " << improvement << "%\n";

    std::cout << report.str();
    if (!out_path.empty()) {
        std::ofstream out(out_path);
        out << report.str();
    }
    fs::remove(tmp_idx);
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return 2;
    }
    std::vector<std::string_view> args;
    for (int i = 2; i < argc; ++i)
        args.emplace_back(argv[i]);

    const std::string_view cmd = argv[1];
    if (cmd == "index")
        return cmd_index(args);
    if (cmd == "query")
        return cmd_query(args);
    if (cmd == "bench")
        return cmd_bench(args);
    usage();
    return 2;
}

# Mini Search Engine (C++)

A from-scratch **information retrieval** engine in modern C++20: positional inverted index, boolean + phrase queries, BM25 top‑k ranking, index serialization, and measurable posting-list optimizations.

**Outcomes (Apple M1, Release, synthetic 5k-doc corpus):**
- Indexes **~107k docs/s**
- BM25 query **p50 ≈ 684 µs**, **p95 ≈ 801 µs**
- Galloping intersection cuts skewed-list **p95 latency by ~95.6%** vs two-pointer
- **55 tests** pass in CI across Linux / macOS / Windows (Debug+Release), plus **ASan+UBSan**, **clang-tidy (Werror)**, fuzz smoke, and a **60s libFuzzer** campaign

## Features

- Boolean queries: `AND` / `OR` / `NOT`, parentheses, juxtaposition = AND
- Phrase search: `"adjacent tokens"` via positional postings
- BM25 ranked retrieval with top‑k heap
- Tokenizer: case folding, punctuation splitting, optional stopwords + Porter stemming
- Query parser with offset + message on syntax errors
- Two-pointer, **galloping**, and **skip-pointer** posting intersection
- Binary index save/load (`MSEI` v2 with delta+varbyte compressed postings); optional **mmap** load
- Optional multithreaded tokenization (`--threads N`) and **synonym rewrite** (`--synonyms`)
- Catch2 tests, fuzz smoke + libFuzzer CI, clang-tidy Werror; benchmark harness with published numbers

## Architecture

```mermaid
flowchart LR
  corpus[Text corpus] --> tok[Tokenizer]
  tok --> idx[Positional inverted index]
  idx --> ser[Binary serialize]
  ser --> disk[index.bin]
  disk --> load[Deserialize]
  qstr[Query string] --> parser[Query parser]
  parser --> ast[AST]
  ast --> boolEval[Boolean / phrase eval]
  ast --> bm25[BM25 ranker]
  load --> boolEval
  load --> bm25
  boolEval --> results[Doc IDs or top-k]
  bm25 --> results
```

**Core data structure:** term → sorted postings `(doc_id, [positions...])`, plus per-doc length and global `N` / `avgdl` for BM25.

## Complexity

| Operation | Time (typical) | Notes |
|-----------|----------------|-------|
| Index build | O(T) | T = total tokens |
| AND (two-pointer) | O(\|A\| + \|B\|) | Baseline |
| AND (galloping) | O(\|short\| · log\|long\|) | Wins when lengths are skewed |
| Phrase | O(candidates · phrase_len) | Position adjacency check |
| BM25 top‑k | O(\|D_q\| · \|q\| + \|D_q\| log k) | Min-heap of size k |
| Serialize / load | O(postings) | v2: delta + varbyte compressed |
| Parallel tokenize | O(T / threads) | Index assembly remains serial |

## Quick start

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure

# Build an index from fixtures (parallel tokenize)
./build/search index build data/fixtures -o index.bin --threads 4

# Boolean / phrase
./build/search query index.bin 'cats AND (milk OR dogs)' --mode boolean --intersect skip
./build/search query index.bin kitten --mode boolean --synonyms data/synonyms.txt --mmap

# Ranked
./build/search query index.bin 'cats milk' --mode bm25 --topk 5

# Benchmarks
./build/mse_bench --docs 5000 --out docs/bench-latest.md
# or: ./scripts/run_benchmarks.sh 5000
```

CMake options: `MSE_BUILD_TESTS`, `MSE_BUILD_BENCH`, `MSE_BUILD_FUZZ`, `MSE_ENABLE_ASAN`, `MSE_ENABLE_UBSAN`, `MSE_ENABLE_CLANG_TIDY`.

## Benchmark results

From [`docs/bench-latest.md`](docs/bench-latest.md) (Apple M1, macOS, Release, seed=42):

| Metric | Value |
|--------|-------|
| Docs | 5,000 |
| Build throughput | ~107,054 docs/s |
| Index on disk | ~1.95 MB |
| BM25 p50 / p95 | 684 µs / 801 µs |
| Intersect two-pointer p95 | 47.1 µs |
| Intersect galloping p95 | 2.1 µs |
| **Galloping p95 improvement** | **~95.6%** |

Methodology: [`docs/performance.md`](docs/performance.md).

## Documentation

- [`docs/query-language.md`](docs/query-language.md) — syntax, precedence, errors
- [`docs/design.md`](docs/design.md) — indexing, ranking, intersection
- [`docs/performance.md`](docs/performance.md) — how numbers are measured
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — completed checklist + stretch goals
- [`CHANGELOG.md`](CHANGELOG.md)

## Project layout

```
include/mse/     Public library headers
src/             Library + CLI (search)
tests/           Catch2 (43 cases)
benchmarks/      mse_bench harness
data/fixtures/   Tiny corpus for demos/tests
scripts/         Corpus prep + benchmark runners
docs/            Design + methodology + latest numbers
```

## Contributing

- C++20, clang-format (LLVM-based [`.clang-format`](.clang-format))
- Keep changes covered by tests; run `ctest` before PRs
- Prefer small, reviewable PRs (IR feature / perf / docs)

## Limitations & roadmap (Phase 2)

- No delta/varbyte posting compression yet
- Indexing is single-threaded
- No Windows CI matrix (Linux + macOS only for now)
- No parser fuzzing / clang-tidy gate in CI yet

## License

Personal portfolio project — see repository for terms.

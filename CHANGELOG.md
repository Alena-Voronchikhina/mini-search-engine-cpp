# Changelog

## [0.4.0] — 2026-08-05

### Added
- Skip-pointer posting lists + `--intersect skip`
- Synonym / query rewrite (`--synonyms file`, see `data/synonyms.txt`)
- mmap index load (`--mmap` / `load_index_mmap`)
- libFuzzer CI campaign (60s) with `fuzz/corpus`
- clang-tidy WarningsAsErrors in CI

## [0.3.0] — 2026-08-05

### Added
- Delta + varbyte posting compression (index format v2; still loads v1)
- Multithreaded corpus tokenization via `search index build --threads N`
- Vendored Catch2 under `third_party/` (CI no longer depends on FetchContent network)
- Parser/tokenizer fuzz smoke binary (`mse_fuzz`)
- clang-tidy CI job
- Windows (`windows-latest`) in the build/test matrix
- `docs/ROADMAP.md`

## [0.2.0] — 2026-08-05

### Added
- C++20 `mse` library: tokenizer, Porter stemmer, positional inverted index
- Boolean query language with AND/OR/NOT, parentheses, and phrase search
- BM25 top-k ranking and CLI modes (`index` / `query` / `bench`)
- Two-pointer and galloping posting intersection
- Binary index serialization (`MSEI`)
- Catch2 test suite and ASan/UBSan CI job
- Linux + macOS Debug/Release GitHub Actions workflow
- Docs: query language, design, performance methodology, published bench numbers

### Removed
- Toy AND-only `Indexer` API and Windows-only starter CMake workflow

## [0.1.0] — 2025-10

- Initial AND-only inverted index demo over two sample documents

# Changelog

## [0.2.0] — 2026-08-05

### Added
- C++20 `mse` library: tokenizer, Porter stemmer, positional inverted index
- Boolean query language with AND/OR/NOT, parentheses, and phrase search
- BM25 top-k ranking and CLI modes (`index` / `query` / `bench`)
- Two-pointer and galloping posting intersection
- Binary index serialization (`MSEI`)
- Catch2 test suite (43 cases) and ASan/UBSan CI job
- Linux + macOS Debug/Release GitHub Actions workflow
- Docs: query language, design, performance methodology, published bench numbers

### Removed
- Toy AND-only `Indexer` API and Windows-only starter CMake workflow

## [0.1.0] — 2025-10

- Initial AND-only inverted index demo over two sample documents

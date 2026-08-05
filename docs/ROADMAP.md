# Roadmap

## Done (v0.2 / v0.3)

- [x] Boolean OR/NOT + parentheses
- [x] Phrase search (positional index)
- [x] BM25 ranking + top-k
- [x] 40+ unit/integration tests
- [x] CI: Linux / macOS / Windows, Debug+Release
- [x] ASan + UBSan CI job
- [x] Benchmark harness + published numbers
- [x] Index save/load
- [x] Galloping intersection (measured vs two-pointer)
- [x] Delta + varbyte posting compression (index format v2)
- [x] Multithreaded corpus tokenization (`--threads`)
- [x] Parser/tokenizer fuzz smoke binary
- [x] Vendored Catch2 (no FetchContent flake)
- [x] clang-tidy CI job

## Next (stretch)

- [ ] Skip pointers stored in postings (not just galloping search)
- [ ] Longer libFuzzer corpus campaigns in CI
- [ ] clang-tidy WarningsAsErrors
- [ ] Query rewrite / synonym expansion
- [ ] Streaming / mmap index for larger corpora

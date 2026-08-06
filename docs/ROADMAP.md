# Roadmap

Track work here and via [GitHub Issues](https://github.com/Alena-Voronchikhina/mini-search-engine-cpp/issues)
(bug / feature templates under `.github/ISSUE_TEMPLATE/`). Releases: `v0.2.0`–`v0.4.0`.

## Done

### v0.2 — Core IR
- [x] Boolean OR/NOT + parentheses
- [x] Phrase search (positional index)
- [x] BM25 ranking + top-k
- [x] 40+ unit/integration tests
- [x] CI: Linux / macOS, Debug+Release
- [x] ASan + UBSan CI job
- [x] Benchmark harness + published numbers
- [x] Index save/load
- [x] Galloping intersection (measured vs two-pointer)

### v0.3 — Hardening
- [x] Delta + varbyte posting compression (index format v2)
- [x] Multithreaded corpus tokenization (`--threads`)
- [x] Parser/tokenizer fuzz smoke binary
- [x] Vendored Catch2 (no FetchContent flake)
- [x] clang-tidy CI job
- [x] Windows matrix

### v0.4 — Stretch complete
- [x] Skip pointers stored on posting lists + skip intersection mode
- [x] libFuzzer 60s CI campaign with seed corpus
- [x] clang-tidy WarningsAsErrors
- [x] Query rewrite / synonym expansion (`--synonyms`)
- [x] mmap-backed index load (`--mmap`)

### Packaging polish
- [x] README limitations match shipped features
- [x] RSS memory footprint in published bench output
- [x] clang-format enforced in CI
- [x] Issue templates + CONTRIBUTING + portfolio status links

## Open / future (file an issue to claim)

- [ ] Persist skip tables inside on-disk index format
- [ ] Query-time synonym injection into BM25 term weights
- [ ] Larger public IR collections (MS MARCO subset) in bench scripts
- [ ] GitHub Project board mirroring this checklist (optional org/user Projects setup)

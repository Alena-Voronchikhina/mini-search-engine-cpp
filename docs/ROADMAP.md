# Roadmap

Also tracked in [Issues](https://github.com/Alena-Voronchikhina/mini-search-engine-cpp/issues).
Releases so far: `v0.2.0`–`v0.4.0`.

## Done

### v0.2 — core IR
- [x] Boolean OR/NOT + parentheses
- [x] Phrase search (positional index)
- [x] BM25 ranking + top-k
- [x] Unit / integration tests
- [x] CI: Linux / macOS, Debug + Release
- [x] ASan + UBSan CI job
- [x] Benchmark harness + published numbers
- [x] Index save/load
- [x] Galloping intersection (measured vs two-pointer)

### v0.3 — hardening
- [x] Delta + varbyte posting compression (index format v2)
- [x] Multithreaded corpus tokenization (`--threads`)
- [x] Parser/tokenizer fuzz smoke binary
- [x] Vendored Catch2
- [x] clang-tidy CI job
- [x] Windows matrix

### v0.4
- [x] Skip pointers on posting lists + skip intersection mode
- [x] libFuzzer CI job with a small seed corpus
- [x] clang-tidy WarningsAsErrors
- [x] Synonym / query rewrite (`--synonyms`)
- [x] mmap-backed index load (`--mmap`)
- [x] RSS in bench output; clang-format in CI; issue templates

## Next

- [ ] Persist skip tables in the on-disk index
- [ ] Feed synonyms into BM25 term weights
- [ ] Bench against a larger public collection (e.g. MS MARCO subset)

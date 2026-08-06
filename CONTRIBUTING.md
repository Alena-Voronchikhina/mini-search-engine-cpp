# Contributing

Thanks for improving this portfolio IR engine. Keep changes small, tested, and formatted.

## Setup

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMSE_BUILD_TESTS=ON -DMSE_BUILD_BENCH=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Coding standards

- **C++20**, no compiler extensions (`CMAKE_CXX_EXTENSIONS=OFF`)
- Format with **clang-format** (`.clang-format`, LLVM-based). CI fails on format drift.
- **clang-tidy** runs with warnings-as-errors in CI (`.clang-tidy`)
- Prefer clear names over cleverness; match existing style in `include/mse/` and `src/`
- Do not vendor unrelated dependencies; Catch2 lives under `third_party/`

## Tests

- Add or extend Catch2 cases under `tests/` for tokenizer, parser, eval, phrase, ranker, serialize, and integration paths
- Tag regressions with `[regression]`
- Run sanitizers locally when touching parsers or memory-sensitive code:

```bash
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug \
  -DMSE_ENABLE_ASAN=ON -DMSE_ENABLE_UBSAN=ON -DMSE_BUILD_BENCH=OFF
cmake --build build-san -j && ctest --test-dir build-san --output-on-failure
```

## Benchmarks

After meaningful performance changes, refresh published numbers:

```bash
./scripts/run_benchmarks.sh 5000 docs/bench-latest.md
```

Update the README benchmark table if headline metrics move.

## Issues and roadmap

- File bugs / features with the GitHub issue templates (`.github/ISSUE_TEMPLATE/`)
- Track larger themes in [`docs/ROADMAP.md`](docs/ROADMAP.md)
- Prefer one concern per PR (IR feature / perf / docs / tooling)

## Pull requests

1. Branch from `main`
2. Keep the diff reviewable
3. Ensure `ctest` and format check pass
4. Summarize what changed and how you verified it

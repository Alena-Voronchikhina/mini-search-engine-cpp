# Contributing

Small, tested, formatted changes are easiest to review.

## Setup

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMSE_BUILD_TESTS=ON -DMSE_BUILD_BENCH=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Style

- C++20, no compiler extensions (`CMAKE_CXX_EXTENSIONS=OFF`)
- Format with clang-format (`.clang-format`). CI fails on drift.
- clang-tidy runs with warnings-as-errors in CI (`.clang-tidy`)
- Match the style in `include/mse/` and `src/`
- Catch2 is under `third_party/` — don’t pull in extra deps without a reason

## Tests

- Add or extend Catch2 cases under `tests/`
- Tag regressions with `[regression]`
- For parser / memory-sensitive changes, run sanitizers:

```bash
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug \
  -DMSE_ENABLE_ASAN=ON -DMSE_ENABLE_UBSAN=ON -DMSE_BUILD_BENCH=OFF
cmake --build build-san -j && ctest --test-dir build-san --output-on-failure
```

## Benchmarks

If you change something performance-related, refresh the published numbers:

```bash
./scripts/run_benchmarks.sh 5000 docs/bench-latest.md
```

Update the README table if the headline metrics move.

## PRs

1. Branch from `main`
2. One concern per PR when you can (feature / perf / docs / tooling)
3. `ctest` + format check green
4. Short note on what changed and how you checked it

Larger ideas live in [`docs/ROADMAP.md`](docs/ROADMAP.md); bugs/features can use the
GitHub issue templates under `.github/ISSUE_TEMPLATE/`.

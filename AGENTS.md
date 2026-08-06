# AGENTS.md

## Cursor Cloud specific instructions

This is a header-only-ish C++20 project built with CMake. It produces a single CLI
(`search`), a test binary (`mse_tests`), a benchmark harness (`mse_bench`), and a fuzz
binary (`mse_fuzz`). There is no package manager and no long-running service — it is a
command-line information-retrieval engine. Catch2 is vendored under `third_party/Catch2`,
so configuring/building needs no network access.

Standard commands live in `README.md` ("Quick start") and `CMakeLists.txt` (options) —
use those as the source of truth. Common flows:

- Configure + build (dev): `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"`
- Test: `ctest --test-dir build --output-on-failure` (55 tests)
- Run app: `./build/search index build data/fixtures -o index.bin --threads 4` then `./build/search query index.bin 'cats AND milk' --mode boolean` (also `--mode bm25`).
- Benchmark: `./build/mse_bench --docs 5000 --out docs/bench-latest.md`
- Fuzz smoke: `./build/mse_fuzz`
- Lint gate (matches CI `clang-tidy` job):
  ```
  cmake -S . -B build-tidy -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DMSE_BUILD_TESTS=OFF -DMSE_BUILD_BENCH=OFF -DMSE_BUILD_FUZZ=OFF
  clang-tidy -p build-tidy --config-file=.clang-tidy --warnings-as-errors='*' $(find src include/mse -name '*.cpp' -o -name '*.hpp')
  ```

### Non-obvious gotchas

- The default `c++`/`cc` compiler alternative on this VM is **clang**, and clang selects
  the GCC 14 toolchain. Both `clang`/`clang++` and `g++` are set up to work (the
  `libstdc++-14-dev` package the clang toolchain needs is installed in the VM image). If
  you ever hit `cannot find -lstdc++`, it means the matching `libstdc++-<gccmajor>-dev` is
  missing for the GCC toolchain clang picked. Plain `g++` uses GCC 13 and works regardless.
- The libFuzzer CI job builds with `clang++` and `-DMSE_FUZZER_LIBFUZGER=ON`; that path
  relies on the clang toolchain above.
- Generated indexes (`*.bin`) and all `build*/` directories are gitignored — running the
  app/benchmarks does not dirty the tree.
- Synonym rewrite only matches when token forms line up: query, index build, and rewrite
  must use consistent `--stem`. Example `search query index.bin kitten --synonyms data/synonyms.txt`
  returns nothing against a non-stemmed index because indexed `cats` != synonym `cat`;
  build and query with `--stem` for it to match.

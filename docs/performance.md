# Performance methodology

## Harness

`mse_bench` (also `search bench`) builds a synthetic corpus with a fixed vocabulary and seed, then reports:

1. **Machine** — OS / distro, CPU, RAM, logical CPUs, compiler (auto-captured when available)
2. **Build throughput** — wall time to tokenize + add + finalize
3. **Index size** — bytes of serialized `index.bin`
4. **RSS after build** — process resident set size (`/proc/self/status` on Linux, Mach task info on macOS; `0` if unavailable)
5. **BM25 latency** — 100 queries cycling a small query set; p50/p95 in microseconds
6. **Intersection microbench** — 200 runs of two-pointer vs galloping on a **skewed** pair:
   - long list: `max(10N, 50_000)` consecutive doc IDs
   - short list: every 997th ID

## Dataset notes

The in-harness corpus is synthetic on purpose: fixed ~26-term vocabulary, document length
uniform in `[20, 80]`, default seed `42`. That keeps runs comparable across machines when
you also record the machine block. It is **not** a retrieval-quality benchmark.

For an on-disk corpus with the same generator:

```bash
./scripts/prepare_corpus.sh data/bench 5000 42
./build/search index build data/bench -o /tmp/bench.bin
```

## Reproduce

```bash
./scripts/bench.sh 5000 docs/bench-latest.md
```

(`scripts/run_benchmarks.sh` is the same runner.) Prefer a Release build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMSE_BUILD_BENCH=ON
cmake --build build -j
```

## Published snapshot

See [`bench-latest.md`](bench-latest.md). Numbers vary by CPU, compiler, and load; always
compare using the machine block in that file. Refresh the README results table when
headline metrics change.

## Interpreting galloping gains

Galloping is not universally faster. On near-equal list lengths, two-pointer often wins. The published improvement targets the classic IR skew (rare term ⊓ common term).

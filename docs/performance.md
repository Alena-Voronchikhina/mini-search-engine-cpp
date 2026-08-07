# Performance methodology

## Harness

`mse_bench` (also `search bench`) builds a synthetic corpus with a fixed vocabulary and seed, then reports:

1. **Build throughput** — wall time to tokenize + add + finalize
2. **Index size** — bytes of serialized `index.bin`
3. **RSS after build** — process resident set size (`/proc/self/status` on Linux, Mach task info on macOS; `0` if unavailable)
4. **BM25 latency** — 100 queries cycling a small query set; p50/p95 in microseconds
5. **Intersection microbench** — 200 runs of two-pointer vs galloping on a **skewed** pair:
   - long list: `max(10N, 50_000)` consecutive doc IDs
   - short list: every 997th ID

Reproduce:

```bash
./scripts/run_benchmarks.sh 5000 docs/bench-latest.md
```

Or prepare an on-disk corpus:

```bash
./scripts/prepare_corpus.sh data/bench 5000 42
./build/search index build data/bench -o /tmp/bench.bin
```

## Published snapshot

See [`bench-latest.md`](bench-latest.md). Numbers vary by CPU, compiler, and load; always record machine + build type when comparing. Refresh the README table when headline metrics change.

## Interpreting galloping gains

Galloping is not universally faster. On near-equal list lengths, two-pointer often wins. The published improvement targets the classic IR skew (rare term ⊓ common term).

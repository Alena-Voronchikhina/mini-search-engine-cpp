# Benchmark results

- Machine:
  - OS: Linux 6.12.94+ (x86_64)
  - Distro: Ubuntu 24.04.4 LTS
  - CPU: Intel(R) Xeon(R) Processor
  - RAM: 16398384 kB
  - Logical CPUs: 4
  - Compiler: Clang 18.1.3
  - Build: CMAKE_BUILD_TYPE=Release (run via scripts/bench.sh)
- Reproduce: `./scripts/bench.sh 5000 docs/bench-latest.md`
- Dataset: synthetic fixed vocab (26 terms), doc length 20–80 tokens, seed=42
- Docs: 5000 (seed=42)
- Build time: 0.1367 s (36576.4 docs/s)
- Index size on disk: 883309 bytes
- RSS after build: 12566528 bytes (~11.9844 MiB)
- BM25 query latency: p50=3640.91 us, p95=4070.88 us
- Intersect two-pointer: p50=498.792 us, p95=532.995 us
- Intersect galloping: p50=6.027 us, p95=7.69605 us
- Galloping vs two-pointer p95 improvement: 98.5561%

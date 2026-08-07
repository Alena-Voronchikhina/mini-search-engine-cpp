# Benchmark results

- Machine: Linux x86_64, Ubuntu 24.04, Clang 18, `CMAKE_BUILD_TYPE=Release`
- Docs: 5000 (seed=42)
- Build time: 0.0357234 s (139964 docs/s)
- Index size on disk: 883309 bytes
- RSS after build: 12349440 bytes (~11.8 MiB)
- BM25 query latency: p50=893.862 us, p95=1029.91 us
- Intersect two-pointer: p50=50.3305 us, p95=50.345 us
- Intersect galloping: p50=1.7845 us, p95=1.7954 us
- Galloping vs two-pointer p95 improvement: 96.4338%

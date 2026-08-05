# Benchmark results

- Machine: Apple M1, macOS 26.6, AppleClang, `CMAKE_BUILD_TYPE=Release`
- Docs: 5000 (seed=42)
- Build time: 0.0467053 s (107054 docs/s)
- Index size on disk: 1954594 bytes
- BM25 query latency: p50=683.688 us, p95=800.877 us
- Intersect two-pointer: p50=46.8125 us, p95=47.083 us
- Intersect galloping: p50=2.0415 us, p95=2.084 us
- Galloping vs two-pointer p95 improvement: 95.5738%

#!/usr/bin/env bash
# Build (if needed) and run the benchmark harness; write docs/performance numbers.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
DOCS="${1:-5000}"
OUT="${2:-$ROOT/docs/bench-latest.md}"

if [[ ! -x build/mse_bench ]]; then
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMSE_BUILD_TESTS=ON -DMSE_BUILD_BENCH=ON
  cmake --build build -j
fi

./build/mse_bench --docs "$DOCS" --out "$OUT"
echo "Wrote $OUT"

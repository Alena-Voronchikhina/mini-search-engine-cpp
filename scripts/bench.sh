#!/usr/bin/env bash
# Canonical benchmark entrypoint (wraps run_benchmarks.sh).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
exec "$ROOT/scripts/run_benchmarks.sh" "${1:-5000}" "${2:-$ROOT/docs/bench-latest.md}"

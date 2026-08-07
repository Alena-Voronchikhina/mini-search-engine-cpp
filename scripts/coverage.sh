#!/usr/bin/env bash
# Build with coverage, run tests, write docs/coverage-latest.md (line %).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/docs/coverage-latest.md}"
BUILD_DIR="${COVERAGE_BUILD_DIR:-build-coverage}"

if ! command -v lcov >/dev/null 2>&1; then
  echo "lcov is required (e.g. sudo apt-get install -y lcov)" >&2
  exit 1
fi

# Prefer GCC: Clang needs compiler-rt profile libs that are often missing.
rm -rf "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=g++ \
  -DMSE_BUILD_TESTS=ON \
  -DMSE_BUILD_BENCH=OFF \
  -DMSE_BUILD_FUZZ=OFF \
  -DMSE_ENABLE_COVERAGE=ON
cmake --build "$BUILD_DIR" --target mse_tests -j
# Run the binary directly (avoids Catch2 NOT_BUILT stubs before rediscovery).
"$BUILD_DIR/mse_tests"

INFO="$BUILD_DIR/coverage.info"
FILTERED="$BUILD_DIR/coverage.filtered.info"
rm -f "$INFO" "$FILTERED"

# Capture only the library objects; ignore noisy gcov/source path issues (lcov 2.x).
LCOV_IGNORE=(--ignore-errors mismatch,gcov,unused,source,empty)
lcov --capture \
  --directory "$BUILD_DIR/CMakeFiles/mse.dir" \
  --output-file "$INFO" \
  --rc branch_coverage=0 \
  --rc geninfo_unexecuted_blocks=1 \
  "${LCOV_IGNORE[@]}"

lcov --extract "$INFO" \
  "${ROOT}/src/*" \
  "${ROOT}/include/mse/*" \
  --output-file "$FILTERED" \
  --rc branch_coverage=0 \
  "${LCOV_IGNORE[@]}"

SUMMARY="$(lcov --summary "$FILTERED" --rc branch_coverage=0 "${LCOV_IGNORE[@]}" 2>&1)"
echo "$SUMMARY"

# Parse "lines......: 87.5% (700 of 800 lines)"
LINE_PCT="$(echo "$SUMMARY" | sed -n 's/.*lines[[:space:].]*:[[:space:]]*\([0-9.]*\)%.*/\1/p' | head -1)"
LINE_COUNTS="$(echo "$SUMMARY" | sed -n 's/.*lines[[:space:].]*:[[:space:]]*[0-9.]*%[[:space:]]*(\([^)]*\)).*/\1/p' | head -1)"
if [[ -z "$LINE_PCT" ]]; then
  echo "Failed to parse lcov line coverage" >&2
  exit 1
fi

# Badge-friendly integer (round half up).
LINE_PCT_INT="$(awk -v p="$LINE_PCT" 'BEGIN { printf "%d", (p + 0.5) }')"

{
  echo "# Coverage"
  echo
  echo "- Tool: lcov + gcov (\`MSE_ENABLE_COVERAGE=ON\`, Debug, g++)"
  echo "- Scope: \`src/\` + \`include/mse/\` (excludes third_party, tests, benchmarks, fuzz)"
  echo "- Line coverage: ${LINE_PCT}% (${LINE_COUNTS})"
  echo "- Reproduce: \`./scripts/coverage.sh\`"
  echo
  echo '```'
  echo "$SUMMARY"
  echo '```'
} >"$OUT"

echo "Wrote $OUT (line coverage ${LINE_PCT}% ≈ ${LINE_PCT_INT}%)"

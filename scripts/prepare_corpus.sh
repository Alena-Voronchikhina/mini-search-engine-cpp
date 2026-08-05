#!/usr/bin/env bash
# Generate a synthetic text corpus for local benchmarking.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/data/bench}"
N="${2:-5000}"
SEED="${3:-42}"

mkdir -p "$OUT"
VOCAB=(cat dog milk love play search engine index query rank token stem score
  document posting phrase boolean vector latency memory disk benchmark gallop
  pointer intersect bm25 porter stopword corpus)

python3 - "$OUT" "$N" "$SEED" <<'PY'
import os, random, sys
out, n, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
vocab = """cat dog milk love play search engine index query rank token stem score
document posting phrase boolean vector latency memory disk benchmark gallop
pointer intersect bm25 porter stopword corpus""".split()
rng = random.Random(seed)
os.makedirs(out, exist_ok=True)
for i in range(n):
    L = rng.randint(20, 80)
    text = " ".join(rng.choice(vocab) for _ in range(L))
    with open(os.path.join(out, f"doc_{i:05d}.txt"), "w", encoding="utf-8") as f:
        f.write(text + "\n")
print(f"Wrote {n} docs to {out}")
PY

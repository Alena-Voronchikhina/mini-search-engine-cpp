# Mini Search Engine (C++)

A small inverted-index search engine over a local text corpus. Documents are tokenized, lowercased, and queried with **AND** semantics (every term must appear).

## Features

- Inverted index: word → sorted document IDs
- Normalized tokens (letters only, case-insensitive)
- AND queries via set intersection
- Interactive REPL or one-shot CLI queries

## Build

Requires CMake 3.16+ and a C++17 compiler.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

From the build directory (so `data/` resolves):

```bash
# Interactive
./search

# One-shot AND query
./search cats
./search cats milk
```

Sample corpus:

| File | Text |
|------|------|
| `data/doc1.txt` | Cats love milk. |
| `data/doc2.txt` | Dogs and cats play. |

`cats` matches both docs; `cats milk` matches only `doc1`.

## Layout

```
include/Indexer.hpp   # Indexer API
src/Indexer.cpp       # Build + query_and
src/main.cpp          # CLI / REPL
data/                 # Sample documents
```

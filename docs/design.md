# Design notes

## Indexing

Documents are tokenized into a stream of terms. For each term occurrence at position `p` in document `d`, the engine appends `p` to the posting for `(term, d)`.

After all documents are added, `finalize()`:

1. Ensures each posting list is sorted by `doc_id`
2. Computes `avgdl` for BM25

## Boolean evaluation

The query AST is evaluated recursively:

- **Term** → document IDs from that term’s posting list
- **Phrase** → candidate intersection of member terms, then position adjacency check (`p, p+1, …`)
- **AND** → posting intersection (galloping by default)
- **OR** → sorted union
- **NOT** → universe (all doc IDs) minus child set

## Intersection algorithms

**Two-pointer:** advance the smaller head; classic merge.

**Galloping:** iterate the shorter list; exponentially search then binary-search into the longer list. Best when `|short| ≪ |long|`.

## BM25

Standard Okapi BM25 with `k1=1.2`, `b=0.75`:

\[
\text{score}(d,q)=\sum_{t\in q}\text{idf}(t)\cdot\frac{tf(t,d)\,(k_1+1)}{tf(t,d)+k_1(1-b+b\cdot dl(d)/avgdl)}
\]

with \(\text{idf}(t)=\ln(1+(N-df+0.5)/(df+0.5))\).

Top‑k uses a size‑`k` min-heap.

**Mode policy:** bag-of-terms trees (terms joined only by AND/juxtaposition) score over the union of term docs. Queries with `OR` / `NOT` / phrases first apply boolean filtering, then BM25 re-rank the candidate set.

## Serialization

File magic `MSEI`, version `1`, little-endian:

1. `#docs`, `avgdl`, doc path + length records
2. `#terms`, then for each term: string, `#postings`, each `(doc_id, #pos, positions…)`

# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

## Benchmarks

SIFT (10,000 base vectors, 128 dimensions, k=10), against Qdrant
(in-memory mode) and Chroma. Ground truth is SIFT's own precomputed
neighbour list, not derived from any of the systems being compared.

| system | ef | build (s) | recall | p50 (µs) | p99 (µs) |
|---|---|---|---|---|---|
| Lattice | 10 | 18.5 | 0.8980 | 237 | 407 |
| Lattice | 50 | 18.5 | 0.9540 | 583 | 799 |
| Lattice | 100 | 19.0 | 0.9590 | 934 | 1234 |
| Qdrant (in-memory) | — | 0.6 | 1.0000 | 2094 | 2621 |
| Chroma | — | 0.4 | 0.9970 | 311 | 361 |

At every ef tested, Lattice beats Qdrant's query latency by 2x or
more while closing most of the recall gap. Chroma is faster on raw
query latency in this run — that's a real result, not omitted.

Where Lattice loses: build time, by a wide margin (18–19s vs under a
second for both competitors). The gap is partly architectural —
Lattice inserts one vector at a time through an exclusive lock, with
no batch-insert path yet — and partly a benchmark-harness limitation,
since Qdrant and Chroma are both inserted here via their bulk APIs.

This is not a like-for-like comparison. Qdrant and Chroma are full
services with networking, persistence policies, filtering, and
metadata support. Lattice is an embedded library. Some of their
latency is buying features Lattice doesn't have. Qdrant runs in
`:memory:` mode here specifically to remove the network hop, as the
closest available approximation to comparing against a library.

Full methodology and raw output in
[bench/results.md](bench/results.md). A full SIFT1M run is in
progress and will be added once complete.

## Status

Storage, search, indexing, quantization, and concurrency all work:

- Append-only write-ahead log with replay
- Memory-mapped segment files for settled data
- Recovery on open: load the segment, replay the WAL on top
- Checkpointing folds the WAL into a new segment via atomic rename
- Brute-force k-nearest-neighbour search over squared L2 distance
- HNSW index: layered graph construction, neighbour pruning, and
  coarse-to-fine search, with recall measured against the exact path
- Scalar quantization: float32 to uint8, 4x smaller, with the recall
  and speed cost measured rather than assumed
- Concurrent query path: many readers alongside a single writer,
  verified clean under the thread sanitizer

Next: Python bindings and an HTTP server, then benchmarking against
Qdrant and Chroma.

## Writing

- [Building an HNSW index from scratch](https://amankarki.hashnode.dev/building-an-hnsw-index-from-scratch)

## Usage

```bash
lattice /path/to/db insert 1 1.0,0.0,0.0
lattice /path/to/db query 1.0,0.0,0.0 5
lattice /path/to/db checkpoint
lattice /path/to/db stats
```

## Building

```bash
cmake -B build
cmake --build build
```

## License

MIT
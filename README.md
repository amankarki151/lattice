# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

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
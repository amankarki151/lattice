# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

## Status

The index works and agrees with exact search:

- Append-only write-ahead log with replay
- Memory-mapped segment files for settled data
- Recovery on open: load the segment, replay the WAL on top
- Checkpointing folds the WAL into a new segment via atomic rename
- Brute-force k-nearest-neighbour search over squared L2 distance
- HNSW index: layered graph construction, neighbour pruning, and
  coarse-to-fine search
- Recall measured against the brute-force path at a range of ef values
- A CLI for inserting, fetching, querying, and checkpointing

Next: quantization, to cut the memory cost per vector.

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
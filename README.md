# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

## Status

The storage layer works end to end:

- Append-only write-ahead log with replay
- Memory-mapped segment files for settled data
- Recovery on open: load the segment, replay the WAL on top
- Checkpointing folds the WAL into a new segment via atomic rename
- Survives being killed mid-write — torn records are dropped, everything
  before them comes back clean

Still no index, so there's nothing to search yet. Brute-force search is next.

## Building

```bash
cmake -B build
cmake --build build
```

## License

MIT
# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

## Status

Early, but the ground floor works:

- In-memory vector store
- Append-only write-ahead log with replay
- Survives being killed mid-write — torn records are dropped, everything
  before them replays clean

No index yet, so search is nonexistent. That's next.

## Building

```bash
cmake -B build
cmake --build build
```

## License

MIT
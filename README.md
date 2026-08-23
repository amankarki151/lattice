# Lattice

An embedded vector database, written from scratch in C++.

Most vector databases are services you run. Lattice is a library you link
against — closer to SQLite than to Postgres. Single node, no cluster, no
network hop unless you want one.

## Status

Early. Nothing here is usable yet.

## Building

```bash
cmake -B build
cmake --build build
```

## License

MIT
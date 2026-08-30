"""Benchmark Lattice against Qdrant and Chroma on SIFT.

Methodology, kept deliberately strict:

  - Every system gets the same base vectors, the same queries, the
    same k. Nothing gets a friendlier dataset.
  - Recall is measured against SIFT's own precomputed ground truth,
    not against whatever the other systems happen to return.
  - Warm-up queries run before timing starts. The first query after a
    build hits cold cache and isn't representative of steady state.
  - p50 and p99 are both reported. A mean hides tail latency, which is
    usually what actually hurts in production.

Known unfairness, stated rather than hidden: Qdrant and Chroma are
full services with networking, persistence policies, filtering, and
metadata support. Lattice is a library. Some of their extra latency
buys features Lattice simply doesn't have.
"""

import argparse
import os
import statistics
import sys
import time
from dataclasses import dataclass, field
from typing import Dict, List, Optional

import numpy as np

from fvecs import read_fvecs, read_ivecs

_BUILD_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "build",
    "bindings",
)
sys.path.insert(0, _BUILD_DIR)


@dataclass
class Result:
    name: str
    build_seconds: float
    recall: float
    p50_us: float
    p99_us: float
    mean_us: float
    queries: int
    notes: str = ""


def recall_at_k(truth_ids: np.ndarray, got_ids: List[int], k: int) -> float:
    """Fraction of the true top-k that actually came back.

    Compares ids, not distances - two different vectors can sit at the
    same distance from a query, so matching on distance would let a
    wrong answer count as correct.
    """
    truth_set = set(int(x) for x in truth_ids[:k])
    if not truth_set:
        return 1.0
    hits = sum(1 for gid in got_ids if gid in truth_set)
    return hits / len(truth_set)


def summarize(name: str, build_s: float, recalls: List[float],
              times_us: List[float], notes: str = "") -> Result:
    times_sorted = sorted(times_us)
    n = len(times_sorted)
    return Result(
        name=name,
        build_seconds=build_s,
        recall=statistics.mean(recalls),
        p50_us=times_sorted[n // 2],
        p99_us=times_sorted[min(n - 1, int(n * 0.99))],
        mean_us=statistics.mean(times_us),
        queries=n,
        notes=notes,
    )


# ---------------------------------------------------------------- lattice

def bench_lattice(base: np.ndarray, queries: np.ndarray, gt: np.ndarray,
                  k: int, ef: int, warmup: int) -> Result:
    import lattice

    cfg = lattice.HnswConfig()
    cfg.M = 16
    cfg.ef_construction = 200

    index = lattice.ConcurrentIndex(cfg)

    t0 = time.perf_counter()
    for i, vec in enumerate(base):
        index.insert(lattice.Vector(i, vec.tolist()))
    build_s = time.perf_counter() - t0

    # Warm up - first queries after a build hit cold cache.
    for i in range(min(warmup, len(queries))):
        index.search(queries[i].tolist(), k, ef)

    recalls: List[float] = []
    times_us: List[float] = []

    for qi, q in enumerate(queries):
        ql = q.tolist()
        t = time.perf_counter()
        hits = index.search(ql, k, ef)
        times_us.append((time.perf_counter() - t) * 1e6)
        recalls.append(recall_at_k(gt[qi], [h.id for h in hits], k))

    return summarize(f"Lattice (ef={ef})", build_s, recalls, times_us,
                     notes="embedded library, no network")


# ----------------------------------------------------------------- qdrant

def bench_qdrant(base: np.ndarray, queries: np.ndarray, gt: np.ndarray,
                 k: int, warmup: int) -> Optional[Result]:
    try:
        from qdrant_client import QdrantClient
        from qdrant_client.models import Distance, PointStruct, VectorParams
    except ImportError:
        print("qdrant-client not installed, skipping")
        return None

    dim = base.shape[1]
    # :memory: mode - no server, no network hop. Closest thing to a
    # like-for-like comparison with an embedded library.
    client = QdrantClient(":memory:")

    client.recreate_collection(
        collection_name="bench",
        vectors_config=VectorParams(size=dim, distance=Distance.EUCLID),
    )

    t0 = time.perf_counter()
    batch = 1000
    for start in range(0, len(base), batch):
        chunk = base[start:start + batch]
        client.upsert(
            collection_name="bench",
            points=[
                PointStruct(id=start + i, vector=v.tolist())
                for i, v in enumerate(chunk)
            ],
        )
    build_s = time.perf_counter() - t0

    for i in range(min(warmup, len(queries))):
        client.query_points(collection_name="bench",
                            query=queries[i].tolist(), limit=k)

    recalls: List[float] = []
    times_us: List[float] = []

    for qi, q in enumerate(queries):
        ql = q.tolist()
        t = time.perf_counter()
        response = client.query_points(collection_name="bench", query=ql,
                                       limit=k)
        times_us.append((time.perf_counter() - t) * 1e6)
        hit_ids = [p.id for p in response.points]
        recalls.append(recall_at_k(gt[qi], hit_ids, k))

    return summarize("Qdrant (in-memory)", build_s, recalls, times_us,
                     notes="full service, extra features Lattice lacks")


# ----------------------------------------------------------------- chroma

def bench_chroma(base: np.ndarray, queries: np.ndarray, gt: np.ndarray,
                 k: int, warmup: int) -> Optional[Result]:
    try:
        import chromadb
    except ImportError:
        print("chromadb not installed, skipping")
        return None

    client = chromadb.EphemeralClient()
    try:
        client.delete_collection("bench")
    except Exception:
        pass

    collection = client.create_collection(
        name="bench",
        metadata={"hnsw:space": "l2"},
    )

    t0 = time.perf_counter()
    batch = 1000
    for start in range(0, len(base), batch):
        chunk = base[start:start + batch]
        collection.add(
            ids=[str(start + i) for i in range(len(chunk))],
            embeddings=[v.tolist() for v in chunk],
        )
    build_s = time.perf_counter() - t0

    for i in range(min(warmup, len(queries))):
        collection.query(query_embeddings=[queries[i].tolist()],
                         n_results=k)

    recalls: List[float] = []
    times_us: List[float] = []

    for qi, q in enumerate(queries):
        ql = q.tolist()
        t = time.perf_counter()
        res = collection.query(query_embeddings=[ql], n_results=k)
        times_us.append((time.perf_counter() - t) * 1e6)
        got = [int(x) for x in res["ids"][0]]
        recalls.append(recall_at_k(gt[qi], got, k))

    return summarize("Chroma", build_s, recalls, times_us,
                     notes="full service, extra features Lattice lacks")


# ------------------------------------------------------------------- main

def print_table(results: List[Result], k: int, base_n: int, dim: int):
    print()
    print(f"SIFT, {base_n} base vectors, dim {dim}, k={k}")
    print()
    header = (f"{'system':<24}{'build s':>10}{'recall':>10}"
              f"{'p50 us':>10}{'p99 us':>10}{'mean us':>10}")
    print(header)
    print("-" * len(header))
    for r in results:
        print(f"{r.name:<24}{r.build_seconds:>10.1f}{r.recall:>10.4f}"
              f"{r.p50_us:>10.0f}{r.p99_us:>10.0f}{r.mean_us:>10.0f}")
    print()
    for r in results:
        if r.notes:
            print(f"  {r.name}: {r.notes}")
    print()


def write_markdown(results: List[Result], k: int, base_n: int, dim: int,
                   path: str):
    lines = [
        "# Benchmark results",
        "",
        "Generated by `bench/run.py`. Numbers are from a single machine "
        "and will vary on yours.",
        "",
        f"Dataset: SIFT, {base_n} base vectors, {dim} dimensions, k={k}.",
        "Ground truth is SIFT's own precomputed neighbour list.",
        "",
        "| system | build (s) | recall | p50 (us) | p99 (us) | mean (us) |",
        "|---|---|---|---|---|---|",
    ]
    for r in results:
        lines.append(
            f"| {r.name} | {r.build_seconds:.1f} | {r.recall:.4f} | "
            f"{r.p50_us:.0f} | {r.p99_us:.0f} | {r.mean_us:.0f} |"
        )

    lines += [
        "",
        "## Caveats",
        "",
        "Qdrant and Chroma are full services with networking, persistence "
        "policies, filtering, and metadata support. Lattice is an embedded "
        "library. Some of their extra latency buys features Lattice does "
        "not have, so this is not a like-for-like comparison and should "
        "not be read as one.",
        "",
        "Qdrant is run in `:memory:` mode to remove the network hop, which "
        "is the closest available approximation to an embedded library.",
        "",
    ]

    with open(path, "w") as f:
        f.write("\n".join(lines))
    print(f"wrote {path}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", default="data/siftsmall")
    ap.add_argument("--prefix", default="siftsmall")
    ap.add_argument("--k", type=int, default=10)
    ap.add_argument("--ef", type=int, default=50)
    ap.add_argument("--warmup", type=int, default=10)
    ap.add_argument("--limit", type=int, default=0,
                    help="cap the number of base vectors, 0 = all")
    ap.add_argument("--out", default="results.md")
    ap.add_argument("--skip", default="",
                    help="comma-separated: qdrant,chroma")
    args = ap.parse_args()

    base = read_fvecs(f"{args.data}/{args.prefix}_base.fvecs")
    queries = read_fvecs(f"{args.data}/{args.prefix}_query.fvecs")
    gt = read_ivecs(f"{args.data}/{args.prefix}_groundtruth.ivecs")

    if args.limit and args.limit < len(base):
        base = base[:args.limit]
        print(f"NOTE: capped to {args.limit} base vectors - ground truth "
              f"is for the full set, so recall will be understated")

    print(f"base {base.shape}, queries {queries.shape}, gt {gt.shape}")

    skip = set(s.strip() for s in args.skip.split(",") if s.strip())
    results: List[Result] = []

    print("\nrunning lattice...")
    results.append(bench_lattice(base, queries, gt, args.k, args.ef,
                                 args.warmup))

    if "qdrant" not in skip:
        print("running qdrant...")
        r = bench_qdrant(base, queries, gt, args.k, args.warmup)
        if r:
            results.append(r)

    if "chroma" not in skip:
        print("running chroma...")
        r = bench_chroma(base, queries, gt, args.k, args.warmup)
        if r:
            results.append(r)

    print_table(results, args.k, len(base), base.shape[1])
    write_markdown(results, args.k, len(base), base.shape[1], args.out)


if __name__ == "__main__":
    main()
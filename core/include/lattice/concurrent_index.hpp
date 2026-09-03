#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <vector>

#include "lattice/hnsw.hpp"
#include "lattice/search.hpp"
#include "lattice/vector.hpp"

namespace lattice {

// Thread-safe wrapper around HnswIndex.
//
// Model: single writer, many readers, guarded by a shared_mutex.
//
// Readers take a shared lock - any number can hold it at once, since
// two threads reading the graph don't conflict with each other. A writer
// takes an exclusive lock, which waits for all current readers to finish
// and blocks new ones until the write is done.
//
// Why not lock-free: inserting into HNSW is a multi-step mutation - add
// the node, add edges in both directions, prune neighbours that went
// over their limit. A reader that catches the graph midway through that
// sequence can walk a half-built node. Making that sequence appear
// atomic without a lock is genuinely hard, and the failure mode is a
// race that only shows up under load. Locks first, measure, then decide.
//
// Known limitation: a writer can starve under constant read load, since
// readers keep renewing the shared lock. Fine for bursty ingestion,
// would need a fairer lock or a write-priority policy otherwise.
class ConcurrentIndex {
public:
    explicit ConcurrentIndex(HnswConfig config = {});

    ConcurrentIndex(const ConcurrentIndex&) = delete;
    ConcurrentIndex& operator=(const ConcurrentIndex&) = delete;

    // Exclusive. Blocks until every in-flight reader is done.
    void insert(const Vector& v);

    void insert_batch(const std::vector<Vector>& vectors);

    // Shared. Any number of threads can be inside this at once.
    std::vector<SearchResult> search(const std::vector<float>& query,
                                     size_t k, size_t ef = 50) const;

    size_t size() const;
    int max_layer() const;

    // Counters for checking the test actually exercised both paths.
    // Relaxed ordering is fine - these are statistics, not
    // synchronization. Nothing branches on them.
    uint64_t insert_count() const {
        return inserts_.load(std::memory_order_relaxed);
    }
    uint64_t search_count() const {
        return searches_.load(std::memory_order_relaxed);
    }

private:
    mutable std::shared_mutex mutex_;
    HnswIndex index_;

    mutable std::atomic<uint64_t> inserts_{0};
    mutable std::atomic<uint64_t> searches_{0};
};

}  // namespace lattice
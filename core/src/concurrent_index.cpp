#include "lattice/concurrent_index.hpp"

#include <mutex>

namespace lattice {

ConcurrentIndex::ConcurrentIndex(HnswConfig config) : index_(config) {}

void ConcurrentIndex::insert(const Vector& v) {
    // unique_lock on a shared_mutex = exclusive. Waits for all current
    // readers to leave, then blocks new ones until this returns.
    std::unique_lock<std::shared_mutex> lock(mutex_);
    index_.insert(v);
    inserts_.fetch_add(1, std::memory_order_relaxed);
}

std::vector<SearchResult> ConcurrentIndex::search(
    const std::vector<float>& query, size_t k, size_t ef) const {
    // shared_lock = read access. Multiple threads hold this at once.
    std::shared_lock<std::shared_mutex> lock(mutex_);
    searches_.fetch_add(1, std::memory_order_relaxed);
    return index_.search(query, k, ef);
}

size_t ConcurrentIndex::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return index_.size();
}

int ConcurrentIndex::max_layer() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return index_.max_layer();
}

}  // namespace lattice
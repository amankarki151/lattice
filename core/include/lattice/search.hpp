#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "lattice/vector.hpp"

namespace lattice {

// One hit from a search. Distance is squared L2, so it's comparable
// between results but isn't a real distance until you sqrt it.
struct SearchResult {
    uint64_t id = 0;
    float distance = 0.0f;
};

// Scans every vector, computes a distance for each, returns the k closest.
//
// This is O(n) per query and always will be. It exists as the correctness
// reference for the HNSW index - if the approximate index disagrees with
// this, the index is wrong. It's also genuinely the right choice for small
// collections, where building a graph costs more than just scanning.
std::vector<SearchResult> brute_force_search(
    const std::vector<Vector>& items,
    const std::vector<float>& query,
    size_t k);

}  // namespace lattice
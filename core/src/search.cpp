#include "lattice/search.hpp"

#include <algorithm>
#include <queue>

#include "lattice/distance.hpp"

namespace lattice {

std::vector<SearchResult> brute_force_search(
    const std::vector<Vector>& items,
    const std::vector<float>& query,
    size_t k) {
    std::vector<SearchResult> out;
    if (items.empty() || k == 0) {
        return out;
    }

    // Max-heap holding the best k found so far, ordered by distance.
    //
    // Max-heap rather than min-heap because the thing you need constant
    // access to is the *worst* of your current best - that's what gets
    // kicked out when something better shows up.
    //
    // Alternative would be scoring everything into a vector and sorting:
    // that's O(n log n) and holds all n results in memory. The heap is
    // O(n log k) and only ever holds k. For k=10 against a million
    // vectors that's a real difference.
    auto worse = [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;
    };
    std::priority_queue<SearchResult, std::vector<SearchResult>,
                        decltype(worse)>
        heap(worse);

    for (const auto& item : items) {
        // Skip anything with a mismatched dimension rather than reading
        // off the end of the shorter one.
        if (item.data.size() != query.size()) {
            continue;
        }

        const float d = l2_squared(item.data, query);

        if (heap.size() < k) {
            heap.push({item.id, d});
        } else if (d < heap.top().distance) {
            heap.pop();
            heap.push({item.id, d});
        }
    }

    // Heap pops worst-first, so fill the output backwards to get
    // nearest-first order.
    out.resize(heap.size());
    for (size_t i = heap.size(); i > 0; --i) {
        out[i - 1] = heap.top();
        heap.pop();
    }

    return out;
}

}  // namespace lattice
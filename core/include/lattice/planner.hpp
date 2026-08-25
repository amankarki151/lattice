#pragma once

#include <cstddef>

namespace lattice {

enum class SearchStrategy {
    Exact,        // scan everything
    Approximate,  // use the HNSW graph - doesn't exist yet
};

// Decides how a query should be answered.
//
// Right now there's only one real option, so this looks like overkill.
// It exists now rather than later because the alternative is retrofitting
// a decision point into every call site once HNSW lands. One function to
// change instead of many.
class QueryPlanner {
public:
    explicit QueryPlanner(size_t exact_threshold = 10000)
        : exact_threshold_(exact_threshold) {}

    // Below the threshold, scanning everything beats walking a graph -
    // the graph traversal overhead costs more than it saves. The actual
    // number here is a placeholder until Day 9 benchmarks give a real one.
    SearchStrategy choose(size_t collection_size, bool index_available) const {
        if (!index_available) {
            return SearchStrategy::Exact;
        }
        if (collection_size < exact_threshold_) {
            return SearchStrategy::Exact;
        }
        return SearchStrategy::Approximate;
    }

    size_t exact_threshold() const { return exact_threshold_; }

private:
    size_t exact_threshold_;
};

}  // namespace lattice
#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <unordered_map>
#include <vector>

#include "lattice/search.hpp"
#include "lattice/vector.hpp"

namespace lattice {

// Hierarchical Navigable Small World graph.
//
// Think of it as a skip list for graphs. Layer 0 holds every vector with
// short-range links between near neighbours. Each layer above holds
// exponentially fewer vectors with longer-range links.
//
// Search enters at the top, greedily walks toward the query until it
// can't improve, drops a layer, repeats. Long jumps first, fine
// refinement last.
//
// Construction is today. Search is tomorrow.

struct HnswConfig {
    size_t M = 16;                 // max neighbours per node, layers 1+
    size_t M_max0 = 32;            // max neighbours at layer 0 (2*M)
    size_t ef_construction = 200;  // search width during insert
    uint64_t seed = 42;            // fixed so builds are reproducible
};

class HnswIndex {
public:
    explicit HnswIndex(HnswConfig config = {});

    void insert(const Vector& v);

    size_t size() const { return nodes_.size(); }
    int max_layer() const { return max_layer_; }
    int64_t entry_point() const { return entry_point_; }

    // How many nodes exist at each layer. Layer 0 first.
    // Used to eyeball whether the level distribution looks sane.
    std::vector<size_t> layer_counts() const;

    // Neighbours of a node at a given layer. For inspection and testing.
    const std::vector<uint64_t>& neighbours(uint64_t id, int layer) const;

private:
    // A node's data plus its links. links_[L] holds its neighbours at
    // layer L. A node in layer 2 has links at layers 0, 1, and 2.
    struct Node {
        Vector vec;
        std::vector<std::vector<uint64_t>> links;
    };

    // Random layer for a new node. Exponentially decaying - most nodes
    // land at 0, a few go higher.
    int random_layer();

    float distance_to(uint64_t id, const std::vector<float>& query) const;

    // Greedy walk at one layer: from entry, keep hopping to whichever
    // neighbour is closer to query, stop when nothing improves.
    uint64_t greedy_closest(uint64_t entry, const std::vector<float>& query,
                            int layer) const;

    // Wider search at one layer. Returns up to ef candidates, closest
    // first. This is what feeds neighbour selection during insert.
    std::vector<SearchResult> search_layer(
        const std::vector<uint64_t>& entry_points,
        const std::vector<float>& query, size_t ef, int layer) const;

    // Picks which of the candidates actually become neighbours.
    std::vector<uint64_t> select_neighbours(
        const std::vector<SearchResult>& candidates, size_t M) const;

    // Adds an edge both ways and trims if a node went over its limit.
    void connect(uint64_t a, uint64_t b, int layer);
    void prune(uint64_t id, int layer, size_t max_links);

    HnswConfig config_;
    std::unordered_map<uint64_t, Node> nodes_;

    int max_layer_ = -1;
    int64_t entry_point_ = -1;  // -1 means the graph is empty

    mutable std::mt19937_64 rng_;
    std::vector<uint64_t> empty_;  // returned when a node has no links
};

}  // namespace lattice
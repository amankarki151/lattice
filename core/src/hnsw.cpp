#include "lattice/hnsw.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>

#include "lattice/distance.hpp"

namespace lattice {

HnswIndex::HnswIndex(HnswConfig config)
    : config_(config), rng_(config.seed) {}

int HnswIndex::random_layer() {
    // The paper's level generator: floor(-ln(uniform(0,1)) * mL), where
    // mL = 1/ln(M). That gives an exponentially decaying distribution -
    // roughly half the nodes at layer 0 only, half of the rest at layer 1,
    // and so on.
    //
    // The point of mL = 1/ln(M) is that it makes the expected number of
    // layers scale with log_M(N), which is what keeps search logarithmic.
    static const double mL = 1.0 / std::log(static_cast<double>(config_.M));

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng_);
    if (r <= 0.0) r = 1e-12;  // guard against log(0)

    return static_cast<int>(-std::log(r) * mL);
}

float HnswIndex::distance_to(uint64_t id,
                             const std::vector<float>& query) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) {
        return std::numeric_limits<float>::max();
    }
    return l2_squared(it->second.vec.data, query);
}

const std::vector<uint64_t>& HnswIndex::neighbours(uint64_t id,
                                                   int layer) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return empty_;
    if (layer < 0 || layer >= static_cast<int>(it->second.links.size())) {
        return empty_;
    }
    return it->second.links[layer];
}

uint64_t HnswIndex::greedy_closest(uint64_t entry,
                                   const std::vector<float>& query,
                                   int layer) const {
    uint64_t current = entry;
    float current_dist = distance_to(current, query);

    // Keep hopping to a closer neighbour until none of them improve.
    // This is the "navigable small world" part - local greedy movement
    // that gets you globally close because of the long-range links.
    bool improved = true;
    while (improved) {
        improved = false;

        for (uint64_t n : neighbours(current, layer)) {
            const float d = distance_to(n, query);
            if (d < current_dist) {
                current = n;
                current_dist = d;
                improved = true;
            }
        }
    }

    return current;
}

std::vector<SearchResult> HnswIndex::search_layer(
    const std::vector<uint64_t>& entry_points,
    const std::vector<float>& query, size_t ef, int layer) const {
    std::unordered_set<uint64_t> visited;

    // Two heaps, deliberately opposite orders.
    //
    // candidates: min-heap. Always gives the closest unexplored node, so
    //   the search expands toward the query first.
    // best: max-heap. Always gives the *worst* of the current results, so
    //   it's cheap to check "is this new node better than my worst?" and
    //   cheap to evict.
    auto closer = [](const SearchResult& a, const SearchResult& b) {
        return a.distance > b.distance;  // min-heap
    };
    auto further = [](const SearchResult& a, const SearchResult& b) {
        return a.distance < b.distance;  // max-heap
    };

    std::priority_queue<SearchResult, std::vector<SearchResult>,
                        decltype(closer)>
        candidates(closer);
    std::priority_queue<SearchResult, std::vector<SearchResult>,
                        decltype(further)>
        best(further);

    for (uint64_t ep : entry_points) {
        if (nodes_.find(ep) == nodes_.end()) continue;
        const float d = distance_to(ep, query);
        candidates.push({ep, d});
        best.push({ep, d});
        visited.insert(ep);
    }

    while (!candidates.empty()) {
        const SearchResult current = candidates.top();

        // If the closest thing left to explore is already worse than the
        // worst result we're keeping, nothing further out can help. Stop.
        if (!best.empty() && current.distance > best.top().distance &&
            best.size() >= ef) {
            break;
        }
        candidates.pop();

        for (uint64_t n : neighbours(current.id, layer)) {
            if (visited.count(n)) continue;
            visited.insert(n);

            const float d = distance_to(n, query);

            if (best.size() < ef) {
                candidates.push({n, d});
                best.push({n, d});
            } else if (d < best.top().distance) {
                candidates.push({n, d});
                best.push({n, d});
                best.pop();
            }
        }
    }

    // Max-heap pops worst-first, so fill backwards for closest-first.
    std::vector<SearchResult> out(best.size());
    for (size_t i = best.size(); i > 0; --i) {
        out[i - 1] = best.top();
        best.pop();
    }
    return out;
}

std::vector<uint64_t> HnswIndex::select_neighbours(
    const std::vector<SearchResult>& candidates, size_t M) const {
    // Simple version: just take the M closest.
    //
    // The paper also describes a heuristic that prefers a diverse spread
    // of neighbours over the raw nearest ones, which builds a better
    // connected graph. Not doing that today - get the simple version
    // working and correct first, then measure whether the heuristic
    // actually helps before adding complexity.
    std::vector<uint64_t> out;
    out.reserve(std::min(M, candidates.size()));

    for (size_t i = 0; i < candidates.size() && out.size() < M; ++i) {
        out.push_back(candidates[i].id);
    }
    return out;
}

void HnswIndex::prune(uint64_t id, int layer, size_t max_links) {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return;
    if (layer >= static_cast<int>(it->second.links.size())) return;

    auto& links = it->second.links[layer];
    if (links.size() <= max_links) return;

    // Over the limit, so keep only the closest max_links neighbours.
    const auto& self = it->second.vec.data;

    std::vector<SearchResult> scored;
    scored.reserve(links.size());
    for (uint64_t n : links) {
        scored.push_back({n, distance_to(n, self)});
    }

    std::sort(scored.begin(), scored.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.distance < b.distance;
              });

    links.clear();
    for (size_t i = 0; i < max_links; ++i) {
        links.push_back(scored[i].id);
    }
}

void HnswIndex::connect(uint64_t a, uint64_t b, int layer) {
    if (a == b) return;

    auto ita = nodes_.find(a);
    auto itb = nodes_.find(b);
    if (ita == nodes_.end() || itb == nodes_.end()) return;

    // Edges are bidirectional. Adding one direction only would leave
    // nodes that can be reached but can't reach back, which breaks
    // greedy traversal in ways that are painful to debug.
    if (layer < static_cast<int>(ita->second.links.size())) {
        auto& la = ita->second.links[layer];
        if (std::find(la.begin(), la.end(), b) == la.end()) {
            la.push_back(b);
        }
    }
    if (layer < static_cast<int>(itb->second.links.size())) {
        auto& lb = itb->second.links[layer];
        if (std::find(lb.begin(), lb.end(), a) == lb.end()) {
            lb.push_back(a);
        }
    }

    const size_t max_links =
        (layer == 0) ? config_.M_max0 : config_.M;
    prune(a, layer, max_links);
    prune(b, layer, max_links);
}

void HnswIndex::insert(const Vector& v) {
    const int layer = random_layer();

    // Create the node with link slots for every layer it lives in.
    Node node;
    node.vec = v;
    node.links.resize(layer + 1);
    nodes_[v.id] = std::move(node);

    // First node ever - it becomes the entry point and there's nothing
    // to connect it to.
    if (entry_point_ < 0) {
        entry_point_ = static_cast<int64_t>(v.id);
        max_layer_ = layer;
        return;
    }

    uint64_t current = static_cast<uint64_t>(entry_point_);

    // Phase 1: from the top down to just above the new node's layer,
    // just navigate. No links get made here - these layers are above
    // the new node so it doesn't belong in them.
    for (int L = max_layer_; L > layer; --L) {
        current = greedy_closest(current, v.data, L);
    }

    // Phase 2: from the new node's layer down to 0, actually connect.
    const int start = std::min(layer, max_layer_);
    for (int L = start; L >= 0; --L) {
        auto candidates =
            search_layer({current}, v.data, config_.ef_construction, L);

        const size_t M = (L == 0) ? config_.M_max0 : config_.M;
        auto chosen = select_neighbours(candidates, M);

        for (uint64_t n : chosen) {
            connect(v.id, n, L);
        }

        // Carry the best entry point down to the next layer rather than
        // restarting from the top each time.
        if (!candidates.empty()) {
            current = candidates.front().id;
        }
    }

    // If this node went higher than anything before it, it becomes the
    // new entry point - search has to start at the top.
    if (layer > max_layer_) {
        max_layer_ = layer;
        entry_point_ = static_cast<int64_t>(v.id);
    }
}

std::vector<size_t> HnswIndex::layer_counts() const {
    std::vector<size_t> counts(max_layer_ + 1, 0);
    for (const auto& [id, node] : nodes_) {
        for (size_t L = 0; L < node.links.size(); ++L) {
            if (L < counts.size()) counts[L]++;
        }
    }
    return counts;
}

}  // namespace lattice
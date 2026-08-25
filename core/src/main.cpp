#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "lattice/hnsw.hpp"
#include "lattice/vector.hpp"

namespace {

constexpr size_t kDim = 8;

std::vector<float> random_vector(std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> v(kDim);
    for (size_t i = 0; i < kDim; ++i) {
        v[i] = dist(rng);
    }
    return v;
}

// Walks layer 0 from the entry point and counts what it can reach.
// If this comes back well under the total, the graph is fragmented and
// search would silently miss whole regions of the data.
size_t reachable_at_layer0(const lattice::HnswIndex& index,
                           const std::vector<uint64_t>& all_ids) {
    if (index.entry_point() < 0) return 0;

    std::unordered_set<uint64_t> seen;
    std::vector<uint64_t> stack;

    const uint64_t start = static_cast<uint64_t>(index.entry_point());
    stack.push_back(start);
    seen.insert(start);

    while (!stack.empty()) {
        const uint64_t current = stack.back();
        stack.pop_back();

        for (uint64_t n : index.neighbours(current, 0)) {
            if (seen.insert(n).second) {
                stack.push_back(n);
            }
        }
    }

    return seen.size();
}

void cmd_build(size_t count) {
    lattice::HnswIndex index;
    std::mt19937 rng(42);

    std::vector<uint64_t> ids;
    ids.reserve(count);

    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = random_vector(rng);
        index.insert(v);
        ids.push_back(v.id);

        if (i % 2000 == 0) {
            std::cout << "inserted " << i << "\n";
        }
    }

    std::cout << "\n--- graph summary ---\n";
    std::cout << "nodes:       " << index.size() << "\n";
    std::cout << "max layer:   " << index.max_layer() << "\n";
    std::cout << "entry point: " << index.entry_point() << "\n\n";

    std::cout << "layer distribution:\n";
    auto counts = index.layer_counts();
    for (size_t L = 0; L < counts.size(); ++L) {
        const double pct =
            100.0 * static_cast<double>(counts[L]) / static_cast<double>(count);
        std::cout << "  layer " << L << ": " << counts[L] << "  ("
                  << std::fixed << std::setprecision(2) << pct << "%)\n";
    }

    // Average degree at layer 0 - should sit somewhere under M_max0.
    size_t total_links = 0;
    size_t isolated = 0;
    for (uint64_t id : ids) {
        const size_t deg = index.neighbours(id, 0).size();
        total_links += deg;
        if (deg == 0) isolated++;
    }

    std::cout << "\nlayer 0 degree:\n";
    std::cout << "  average:  " << std::fixed << std::setprecision(2)
              << (static_cast<double>(total_links) / static_cast<double>(count))
              << "\n";
    std::cout << "  isolated: " << isolated << "\n";

    const size_t reachable = reachable_at_layer0(index, ids);
    std::cout << "\nreachable from entry at layer 0: " << reachable << " / "
              << count << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: scratch build <count>\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "build") {
        cmd_build(argc >= 3 ? std::stoull(argv[2]) : 10000);
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
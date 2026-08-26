#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "lattice/hnsw.hpp"
#include "lattice/search.hpp"
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

// Of the k ids brute force found, how many did HNSW also find?
//
// Compares ids, not distances. Two different vectors can sit at the same
// distance from a query, so matching on distance would call a wrong answer
// correct. Matching on id can't.
double recall_at_k(const std::vector<lattice::SearchResult>& truth,
                   const std::vector<lattice::SearchResult>& got) {
    if (truth.empty()) return 1.0;

    std::unordered_set<uint64_t> truth_ids;
    for (const auto& r : truth) {
        truth_ids.insert(r.id);
    }

    size_t hits = 0;
    for (const auto& r : got) {
        if (truth_ids.count(r.id)) {
            hits++;
        }
    }

    return static_cast<double>(hits) / static_cast<double>(truth.size());
}

struct Dataset {
    lattice::HnswIndex index;
    std::vector<lattice::Vector> flat;
};

Dataset build_dataset(size_t count, size_t ef_construction) {
    lattice::HnswConfig cfg;
    cfg.ef_construction = ef_construction;

    Dataset ds{lattice::HnswIndex(cfg), {}};
    ds.flat.reserve(count);

    std::mt19937 rng(42);
    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = random_vector(rng);

        ds.index.insert(v);
        ds.flat.push_back(v);
    }

    return ds;
}

void cmd_recall(size_t count, size_t k, size_t queries) {
    std::cout << "building index with " << count << " vectors...\n";
    auto ds = build_dataset(count, 200);
    std::cout << "built. max layer " << ds.index.max_layer() << "\n\n";

    // Query vectors from a different seed, so they aren't stored vectors.
    std::mt19937 qrng(1337);

    const std::vector<size_t> ef_values = {10, 20, 50, 100, 200};

    std::cout << "recall@" << k << " over " << queries << " queries\n";
    std::cout << std::left << std::setw(8) << "ef" << std::setw(12) << "recall"
              << std::setw(16) << "hnsw us/query" << "brute us/query\n";
    std::cout << std::string(52, '-') << "\n";

    for (size_t ef : ef_values) {
        if (ef < k) continue;

        double total_recall = 0.0;
        long long hnsw_us = 0;
        long long brute_us = 0;

        std::mt19937 run_rng(1337);  // same queries for every ef

        for (size_t q = 0; q < queries; ++q) {
            auto query = random_vector(run_rng);

            auto t0 = std::chrono::steady_clock::now();
            auto truth = lattice::brute_force_search(ds.flat, query, k);
            auto t1 = std::chrono::steady_clock::now();
            auto got = ds.index.search(query, k, ef);
            auto t2 = std::chrono::steady_clock::now();

            brute_us += std::chrono::duration_cast<std::chrono::microseconds>(
                            t1 - t0).count();
            hnsw_us += std::chrono::duration_cast<std::chrono::microseconds>(
                           t2 - t1).count();

            total_recall += recall_at_k(truth, got);
        }

        const double avg_recall = total_recall / static_cast<double>(queries);

        std::cout << std::left << std::setw(8) << ef << std::setw(12)
                  << std::fixed << std::setprecision(4) << avg_recall
                  << std::setw(16)
                  << (hnsw_us / static_cast<long long>(queries))
                  << (brute_us / static_cast<long long>(queries)) << "\n";
    }
    (void)qrng;
}

// Self-match test: query with a vector that IS in the index. It must come
// back first with distance 0. If this fails, nothing else matters.
void cmd_selfmatch(size_t count, size_t samples) {
    auto ds = build_dataset(count, 200);

    std::mt19937 pick(7);
    std::uniform_int_distribution<size_t> idx(0, ds.flat.size() - 1);

    size_t passed = 0;
    size_t failed = 0;

    for (size_t i = 0; i < samples; ++i) {
        const auto& target = ds.flat[idx(pick)];
        auto hits = ds.index.search(target.data, 1, 50);

        if (!hits.empty() && hits[0].id == target.id &&
            hits[0].distance == 0.0f) {
            passed++;
        } else {
            failed++;
            if (failed <= 5) {
                std::cout << "  MISS: queried id " << target.id << ", got ";
                if (hits.empty()) {
                    std::cout << "nothing\n";
                } else {
                    std::cout << "id " << hits[0].id << " dist "
                              << hits[0].distance << "\n";
                }
            }
        }
    }

    std::cout << "self-match: " << passed << " passed, " << failed
              << " failed, out of " << samples << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage:\n"
                  << "  scratch recall <count> <k> <queries>\n"
                  << "  scratch selfmatch <count> <samples>\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "recall") {
        const size_t count = argc >= 3 ? std::stoull(argv[2]) : 10000;
        const size_t k = argc >= 4 ? std::stoull(argv[3]) : 10;
        const size_t queries = argc >= 5 ? std::stoull(argv[4]) : 100;
        cmd_recall(count, k, queries);
    } else if (cmd == "selfmatch") {
        const size_t count = argc >= 3 ? std::stoull(argv[2]) : 10000;
        const size_t samples = argc >= 4 ? std::stoull(argv[3]) : 200;
        cmd_selfmatch(count, samples);
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
#include <gtest/gtest.h>

#include <random>
#include <unordered_set>
#include <vector>

#include "lattice/hnsw.hpp"
#include "lattice/search.hpp"

#include "lattice/concurrent_index.hpp"
namespace {

std::vector<lattice::Vector> random_dataset(size_t count, size_t dim,
                                            uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<lattice::Vector> out;
    out.reserve(count);
    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data.resize(dim);
        for (size_t d = 0; d < dim; ++d) v.data[d] = dist(rng);
        out.push_back(std::move(v));
    }
    return out;
}

TEST(HnswTest, EmptyIndexReturnsNothing) {
    lattice::HnswIndex index;
    auto hits = index.search({1.0f, 0.0f}, 5);
    EXPECT_TRUE(hits.empty());
}

TEST(HnswTest, SingleVectorIsFound) {
    lattice::HnswIndex index;
    lattice::Vector v;
    v.id = 42;
    v.data = {1.0f, 2.0f, 3.0f};
    index.insert(v);

    auto hits = index.search({1.0f, 2.0f, 3.0f}, 1);
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_EQ(hits[0].id, 42u);
    EXPECT_FLOAT_EQ(hits[0].distance, 0.0f);
}

// Cheapest meaningful correctness check: a vector that's in the index
// must come back first, at distance zero.
TEST(HnswTest, SelfMatchAlwaysSucceeds) {
    auto data = random_dataset(1000, 8, 42);

    lattice::HnswIndex index;
    for (const auto& v : data) index.insert(v);

    for (size_t i = 0; i < data.size(); i += 50) {
        auto hits = index.search(data[i].data, 1, 50);
        ASSERT_FALSE(hits.empty());
        EXPECT_EQ(hits[0].id, data[i].id);
        EXPECT_FLOAT_EQ(hits[0].distance, 0.0f);
    }
}

// No node may be isolated at layer 0 - an isolated node can never be
// reached by search, so it's effectively lost data.
TEST(HnswTest, NoIsolatedNodesAtLayerZero) {
    auto data = random_dataset(500, 8, 7);

    lattice::HnswIndex index;
    for (const auto& v : data) index.insert(v);

    size_t isolated = 0;
    for (const auto& v : data) {
        if (index.neighbours(v.id, 0).empty()) isolated++;
    }
    EXPECT_EQ(isolated, 0u);
}

// The graph must be one connected component at layer 0. A fragmented
// graph doesn't crash - it silently returns bad results, which is
// worse than an obvious failure.
TEST(HnswTest, GraphIsFullyReachableAtLayerZero) {
    auto data = random_dataset(500, 8, 11);

    lattice::HnswIndex index;
    for (const auto& v : data) index.insert(v);

    ASSERT_GE(index.entry_point(), 0);

    std::unordered_set<uint64_t> seen;
    std::vector<uint64_t> stack{static_cast<uint64_t>(index.entry_point())};
    seen.insert(stack.front());

    while (!stack.empty()) {
        const uint64_t current = stack.back();
        stack.pop_back();
        for (uint64_t n : index.neighbours(current, 0)) {
            if (seen.insert(n).second) stack.push_back(n);
        }
    }

    EXPECT_EQ(seen.size(), data.size());
}

// The real correctness bar: HNSW must broadly agree with an exact scan.
TEST(HnswTest, RecallAgainstBruteForceIsHigh) {
    auto data = random_dataset(2000, 8, 99);

    lattice::HnswIndex index;
    for (const auto& v : data) index.insert(v);

    std::mt19937 qrng(1337);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const size_t k = 10;
    const size_t queries = 50;
    double total_recall = 0.0;

    for (size_t q = 0; q < queries; ++q) {
        std::vector<float> query(8);
        for (auto& x : query) x = dist(qrng);

        auto truth = lattice::brute_force_search(data, query, k);
        auto got = index.search(query, k, 100);

        std::unordered_set<uint64_t> truth_ids;
        for (const auto& r : truth) truth_ids.insert(r.id);

        size_t hits = 0;
        for (const auto& r : got) {
            if (truth_ids.count(r.id)) hits++;
        }
        total_recall += static_cast<double>(hits) /
                        static_cast<double>(truth.size());
    }

    const double recall = total_recall / static_cast<double>(queries);
    EXPECT_GT(recall, 0.90) << "recall was " << recall;
}

TEST(HnswTest, EfIsRaisedToKWhenTooSmall) {
    auto data = random_dataset(200, 8, 3);

    lattice::HnswIndex index;
    for (const auto& v : data) index.insert(v);

    // Asking for 20 results with ef=5 shouldn't return only 5.
    auto hits = index.search(data[0].data, 20, 5);
    EXPECT_EQ(hits.size(), 20u);
}

TEST(HnswTest, BatchInsertMatchesIndividualInserts) {
    auto data = random_dataset(500, 8, 55);

    lattice::HnswIndex individual;
    for (const auto& v : data) individual.insert(v);

    lattice::ConcurrentIndex batched;
    batched.insert_batch(data);

    EXPECT_EQ(batched.size(), individual.size());

    for (size_t i = 0; i < data.size(); i += 25) {
        auto hits = batched.search(data[i].data, 1, 50);
        ASSERT_FALSE(hits.empty());
        EXPECT_EQ(hits[0].id, data[i].id);
        EXPECT_FLOAT_EQ(hits[0].distance, 0.0f);
    }
}

}  // namespace
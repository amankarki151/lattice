#include <gtest/gtest.h>

#include <vector>

#include "lattice/distance.hpp"
#include "lattice/search.hpp"

namespace {

lattice::Vector vec(uint64_t id, std::vector<float> data) {
    lattice::Vector v;
    v.id = id;
    v.data = std::move(data);
    return v;
}

TEST(DistanceTest, IdenticalVectorsAreZeroApart) {
    std::vector<float> a{1.0f, 2.0f, 3.0f};
    EXPECT_FLOAT_EQ(lattice::l2_squared(a, a), 0.0f);
}

// Hand-checkable: (1-0)^2 + (0-1)^2 + (0-0)^2 = 2
TEST(DistanceTest, MatchesHandCalculation) {
    std::vector<float> a{1.0f, 0.0f, 0.0f};
    std::vector<float> b{0.0f, 1.0f, 0.0f};
    EXPECT_FLOAT_EQ(lattice::l2_squared(a, b), 2.0f);
}

TEST(BruteForceTest, EmptyCollectionReturnsNothing) {
    auto hits = lattice::brute_force_search({}, {1.0f, 0.0f}, 5);
    EXPECT_TRUE(hits.empty());
}

TEST(BruteForceTest, SelfMatchComesBackFirstAtZero) {
    std::vector<lattice::Vector> items{
        vec(1, {1.0f, 0.0f, 0.0f}),
        vec(2, {0.0f, 1.0f, 0.0f}),
        vec(3, {0.9f, 0.1f, 0.0f}),
    };

    auto hits = lattice::brute_force_search(items, {1.0f, 0.0f, 0.0f}, 3);
    ASSERT_EQ(hits.size(), 3u);
    EXPECT_EQ(hits[0].id, 1u);
    EXPECT_FLOAT_EQ(hits[0].distance, 0.0f);
}

TEST(BruteForceTest, ResultsAreSortedNearestFirst) {
    std::vector<lattice::Vector> items{
        vec(1, {1.0f, 0.0f, 0.0f}),
        vec(2, {0.0f, 1.0f, 0.0f}),
        vec(3, {0.9f, 0.1f, 0.0f}),
    };

    auto hits = lattice::brute_force_search(items, {1.0f, 0.0f, 0.0f}, 3);
    ASSERT_EQ(hits.size(), 3u);
    EXPECT_EQ(hits[0].id, 1u);
    EXPECT_EQ(hits[1].id, 3u);
    EXPECT_EQ(hits[2].id, 2u);

    for (size_t i = 1; i < hits.size(); ++i) {
        EXPECT_LE(hits[i - 1].distance, hits[i].distance);
    }
}

TEST(BruteForceTest, RespectsK) {
    std::vector<lattice::Vector> items;
    for (uint64_t i = 1; i <= 20; ++i) {
        items.push_back(vec(i, {static_cast<float>(i), 0.0f}));
    }

    auto hits = lattice::brute_force_search(items, {1.0f, 0.0f}, 5);
    EXPECT_EQ(hits.size(), 5u);
}

TEST(BruteForceTest, MismatchedDimensionsAreSkipped) {
    std::vector<lattice::Vector> items{
        vec(1, {1.0f, 0.0f, 0.0f}),
        vec(2, {1.0f, 0.0f}),  // wrong dim
    };

    auto hits = lattice::brute_force_search(items, {1.0f, 0.0f, 0.0f}, 5);
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_EQ(hits[0].id, 1u);
}

}  // namespace
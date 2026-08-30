#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "lattice/quantizer.hpp"

namespace {

std::vector<lattice::Vector> uniform_dataset(size_t count, size_t dim,
                                             uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<lattice::Vector> out;
    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data.resize(dim);
        for (size_t d = 0; d < dim; ++d) v.data[d] = dist(rng);
        out.push_back(std::move(v));
    }
    return out;
}

TEST(QuantizerTest, EncodeBeforeTrainThrows) {
    lattice::ScalarQuantizer q;
    EXPECT_THROW(q.encode({1.0f, 2.0f}), std::runtime_error);
}

TEST(QuantizerTest, TrainOnEmptySampleThrows) {
    lattice::ScalarQuantizer q;
    EXPECT_THROW(q.train({}), std::runtime_error);
}

TEST(QuantizerTest, OneByteBerDimension) {
    auto data = uniform_dataset(100, 16, 5);
    lattice::ScalarQuantizer q;
    q.train(data);

    auto encoded = q.encode(data[0].data);
    EXPECT_EQ(encoded.size(), 16u);
}

// Error per dimension can't exceed half a bucket width - that's the
// theoretical bound from rounding to one of 256 evenly spaced points.
TEST(QuantizerTest, ErrorStaysWithinTheoreticalBound) {
    auto data = uniform_dataset(1000, 8, 13);
    lattice::ScalarQuantizer q;
    q.train(data);

    for (const auto& v : data) {
        auto decoded = q.decode(q.encode(v.data));
        for (size_t i = 0; i < v.data.size(); ++i) {
            const float err = std::abs(v.data[i] - decoded[i]);
            const float bound = q.scales()[i] / 2.0f;
            // Small epsilon for float comparison at the boundary.
            EXPECT_LE(err, bound + 1e-6f);
        }
    }
}

TEST(QuantizerTest, ValuesOutsideTrainedRangeAreClamped) {
    auto data = uniform_dataset(100, 4, 21);
    lattice::ScalarQuantizer q;
    q.train(data);

    // Way outside [0,1] in both directions.
    auto high = q.encode({100.0f, 100.0f, 100.0f, 100.0f});
    auto low = q.encode({-100.0f, -100.0f, -100.0f, -100.0f});

    for (uint8_t b : high) EXPECT_EQ(b, 255);
    for (uint8_t b : low) EXPECT_EQ(b, 0);
}

TEST(QuantizerTest, DistanceToQueryMatchesDecodeThenCompute) {
    auto data = uniform_dataset(50, 8, 31);
    lattice::ScalarQuantizer q;
    q.train(data);

    std::vector<float> query(8, 0.5f);

    for (const auto& v : data) {
        auto code = q.encode(v.data);
        auto decoded = q.decode(code);

        float manual = 0.0f;
        for (size_t i = 0; i < decoded.size(); ++i) {
            const float d = decoded[i] - query[i];
            manual += d * d;
        }

        EXPECT_NEAR(q.distance_to_query(code, query), manual, 1e-4f);
    }
}

TEST(QuantizerTest, ZeroRangeDimensionDoesNotDivideByZero) {
    std::vector<lattice::Vector> data;
    for (uint64_t i = 1; i <= 10; ++i) {
        lattice::Vector v;
        v.id = i;
        v.data = {1.0f, static_cast<float>(i)};  // dim 0 is constant
        data.push_back(v);
    }

    lattice::ScalarQuantizer q;
    q.train(data);

    auto decoded = q.decode(q.encode(data[0].data));
    EXPECT_FLOAT_EQ(decoded[0], 1.0f);
}

}  // namespace
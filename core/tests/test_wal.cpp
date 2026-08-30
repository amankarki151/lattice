#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <string>

#include "lattice/wal.hpp"

namespace {

const std::string kPath = "/tmp/lattice_test_wal.wal";

lattice::Vector make(uint64_t id, size_t dim = 4) {
    lattice::Vector v;
    v.id = id;
    v.data.resize(dim);
    for (size_t i = 0; i < dim; ++i) {
        v.data[i] = static_cast<float>(id) * static_cast<float>(i + 1);
    }
    return v;
}

class WalTest : public ::testing::Test {
protected:
    void SetUp() override { std::remove(kPath.c_str()); }
    void TearDown() override { std::remove(kPath.c_str()); }
};

TEST_F(WalTest, ReplayOnEmptyFileReturnsNothing) {
    auto items = lattice::Wal::replay(kPath);
    EXPECT_TRUE(items.empty());
}

TEST_F(WalTest, SingleRecordRoundTrips) {
    {
        lattice::Wal wal(kPath);
        wal.append(make(1));
    }

    auto items = lattice::Wal::replay(kPath);
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].id, 1u);
    EXPECT_EQ(items[0].data.size(), 4u);
    EXPECT_FLOAT_EQ(items[0].data[0], 1.0f);
}

TEST_F(WalTest, ManyRecordsReplayInOrder) {
    {
        lattice::Wal wal(kPath);
        for (uint64_t i = 1; i <= 100; ++i) {
            wal.append(make(i));
        }
    }

    auto items = lattice::Wal::replay(kPath);
    ASSERT_EQ(items.size(), 100u);
    for (size_t i = 0; i < items.size(); ++i) {
        EXPECT_EQ(items[i].id, i + 1);
    }
}

// The behaviour the whole WAL exists for: a torn write at the end
// must not corrupt or lose the records written before it.
TEST_F(WalTest, TornFinalRecordIsDroppedButEarlierOnesSurvive) {
    {
        lattice::Wal wal(kPath);
        for (uint64_t i = 1; i <= 10; ++i) {
            wal.append(make(i));
        }
    }

    // Chop the last few bytes off, simulating a process death mid-write.
    FILE* f = std::fopen(kPath.c_str(), "rb");
    ASSERT_NE(f, nullptr);
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fclose(f);

    ASSERT_EQ(truncate(kPath.c_str(), size - 6), 0);

    auto items = lattice::Wal::replay(kPath);
    EXPECT_EQ(items.size(), 9u);
    for (size_t i = 0; i < items.size(); ++i) {
        EXPECT_EQ(items[i].id, i + 1);
    }
}

TEST_F(WalTest, AppendsRatherThanOverwrites) {
    {
        lattice::Wal wal(kPath);
        wal.append(make(1));
    }
    {
        lattice::Wal wal(kPath);
        wal.append(make(2));
    }

    auto items = lattice::Wal::replay(kPath);
    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0].id, 1u);
    EXPECT_EQ(items[1].id, 2u);
}

}  // namespace
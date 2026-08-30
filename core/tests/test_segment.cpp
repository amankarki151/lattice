#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "lattice/segment.hpp"

namespace {

const std::string kPath = "/tmp/lattice_test_segment.seg";

lattice::Vector make(uint64_t id, size_t dim = 4) {
    lattice::Vector v;
    v.id = id;
    v.data.resize(dim);
    for (size_t i = 0; i < dim; ++i) {
        v.data[i] = static_cast<float>(id) + static_cast<float>(i) * 0.5f;
    }
    return v;
}

class SegmentTest : public ::testing::Test {
protected:
    void SetUp() override { std::remove(kPath.c_str()); }
    void TearDown() override { std::remove(kPath.c_str()); }
};

TEST_F(SegmentTest, MissingFileIsNotAnError) {
    lattice::SegmentReader reader(kPath);
    EXPECT_FALSE(reader.ok());
}

TEST_F(SegmentTest, RoundTripsAllRecords) {
    std::vector<lattice::Vector> items;
    for (uint64_t i = 1; i <= 50; ++i) items.push_back(make(i));

    lattice::SegmentWriter::write(kPath, items);

    lattice::SegmentReader reader(kPath);
    ASSERT_TRUE(reader.ok());
    EXPECT_EQ(reader.count(), 50u);

    auto read = reader.read_all();
    ASSERT_EQ(read.size(), 50u);
    for (size_t i = 0; i < read.size(); ++i) {
        EXPECT_EQ(read[i].id, i + 1);
        EXPECT_FLOAT_EQ(read[i].data[0], static_cast<float>(i + 1));
    }
}

// The magic number exists so a corrupt file fails loudly instead of
// being read as garbage. This is the test that proves it does.
TEST_F(SegmentTest, CorruptMagicIsRejected) {
    std::vector<lattice::Vector> items{make(1), make(2)};
    lattice::SegmentWriter::write(kPath, items);

    // Stomp the first four bytes.
    std::fstream f(kPath, std::ios::binary | std::ios::in | std::ios::out);
    ASSERT_TRUE(f);
    const char junk[4] = {'X', 'X', 'X', 'X'};
    f.seekp(0);
    f.write(junk, 4);
    f.close();

    lattice::SegmentReader reader(kPath);
    EXPECT_FALSE(reader.ok());
}

TEST_F(SegmentTest, EmptySegmentIsValid) {
    lattice::SegmentWriter::write(kPath, {});

    lattice::SegmentReader reader(kPath);
    ASSERT_TRUE(reader.ok());
    EXPECT_EQ(reader.count(), 0u);
    EXPECT_TRUE(reader.read_all().empty());
}

}  // namespace
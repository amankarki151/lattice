#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <unordered_set>

#include "lattice/database.hpp"

namespace {

const std::string kDir = "/tmp/lattice_test_db";

lattice::Vector make(uint64_t id, size_t dim = 4) {
    lattice::Vector v;
    v.id = id;
    v.data.resize(dim);
    for (size_t i = 0; i < dim; ++i) {
        v.data[i] = static_cast<float>(id) * static_cast<float>(i + 1);
    }
    return v;
}

class DatabaseSearchTest : public ::testing::Test {
protected:
    void SetUp() override { std::filesystem::remove_all(kDir); }
    void TearDown() override { std::filesystem::remove_all(kDir); }
};

TEST_F(DatabaseSearchTest, NoFilterReturnsEverything) {
    lattice::Database db(kDir);
    db.insert(make(1));
    db.insert(make(2));
    db.insert(make(3));

    auto hits = db.search(make(1).data, 10);
    EXPECT_EQ(hits.size(), 3u);
}

TEST_F(DatabaseSearchTest, FilterOnlyReturnsIdsInTheSet) {
    lattice::Database db(kDir);
    db.insert(make(1));
    db.insert(make(2));
    db.insert(make(3));

    std::unordered_set<uint64_t> allowed{1, 3};
    auto hits = db.search(make(1).data, 10, allowed);

    ASSERT_EQ(hits.size(), 2u);
    for (const auto& h : hits) {
        EXPECT_TRUE(h.id == 1u || h.id == 3u);
    }
}

// This is the one that would have silently passed before the empty-set
// check landed: an empty filter must mean "nothing is allowed", not
// "no filter was given".
TEST_F(DatabaseSearchTest, EmptyFilterReturnsNothing) {
    lattice::Database db(kDir);
    db.insert(make(1));
    db.insert(make(2));

    std::unordered_set<uint64_t> empty;
    auto hits = db.search(make(1).data, 10, empty);

    EXPECT_TRUE(hits.empty());
}

}  // namespace

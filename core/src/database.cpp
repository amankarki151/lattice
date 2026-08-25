#include "lattice/database.hpp"

#include <sys/stat.h>

#include <cstdio>
#include <iostream>

#include "lattice/segment.hpp"
#include "lattice/search.hpp"

namespace lattice {

namespace {

void ensure_dir(const std::string& dir) {
    // 0755: owner can write, everyone can read and enter. Fails harmlessly
    // if the directory already exists, which is the normal case.
    ::mkdir(dir.c_str(), 0755);
}

}  // namespace

Database::Database(const std::string& dir)
    : dir_(dir),
      seg_path_(dir + "/lattice.seg"),
      wal_path_(dir + "/lattice.wal") {
    ensure_dir(dir_);
    recover();
    wal_ = std::make_unique<Wal>(wal_path_);
}

Database::~Database() = default;

void Database::recover() {
    size_t from_segment = 0;
    size_t from_wal = 0;

    // 1. Load the settled data.
    {
        SegmentReader reader(seg_path_);
        if (reader.ok()) {
            auto items = reader.read_all();
            from_segment = items.size();
            for (const auto& v : items) {
                store_.insert(v);
            }
        }
    }

    // 2. Layer everything written since on top. Same id in both means the
    //    WAL version wins, because it's the newer write.
    {
        auto items = Wal::replay(wal_path_);
        from_wal = items.size();
        for (const auto& v : items) {
            store_.insert(v);
        }
    }

    std::cout << "recovered: " << from_segment << " from segment, "
              << from_wal << " from wal, " << store_.size() << " total\n";
}

void Database::insert(const Vector& v) {
    // Log first, then update memory. If this crashes between the two, the
    // record is still on disk and comes back on the next recover().
    wal_->append(v);
    store_.insert(v);
}

std::optional<Vector> Database::get(uint64_t id) const {
    return store_.get(id);
}

size_t Database::size() const {
    return store_.size();
}

std::vector<SearchResult> Database::search(const std::vector<float>& query,
                                           size_t k) const {
    const auto strategy = planner_.choose(store_.size(), index_available_);

    switch (strategy) {
        case SearchStrategy::Exact:
            // Copies every vector out of the store to scan them, which is
            // wasteful and gets fixed once there's a real index. Today it
            // just needs to be correct - this is what HNSW gets checked
            // against.
            return brute_force_search(store_.snapshot(), query, k);

        case SearchStrategy::Approximate:
            // Not reachable yet - index_available_ is always false until
            // HNSW exists. Falling back rather than throwing so this can't
            // break anything if the flag gets set early by mistake.
            return brute_force_search(store_.snapshot(), query, k);
    }

    return {};
}

void Database::checkpoint() {
    // Snapshot everything currently in memory into a fresh segment.
    std::vector<Vector> items;
    items.reserve(store_.size());
    for (const auto& v : store_.snapshot()) {
        items.push_back(v);
    }

    SegmentWriter::write(seg_path_, items);

    // Segment now covers everything the WAL was holding, so the log can go.
    // Order matters: segment lands first (via atomic rename), only then is
    // the WAL dropped. Crash in between and you just replay a WAL whose
    // contents are already in the segment — harmless, since inserts by id
    // are idempotent.
    wal_.reset();
    std::remove(wal_path_.c_str());
    wal_ = std::make_unique<Wal>(wal_path_);

    std::cout << "checkpoint: wrote " << items.size()
              << " records to segment, wal cleared\n";
}

}  // namespace lattice
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "lattice/flat_store.hpp"
#include "lattice/vector.hpp"
#include "lattice/wal.hpp"

namespace lattice {

// Ties the storage pieces together.
//
// Segment = data that's been settled. WAL = everything written since the
// last checkpoint. Opening the database means loading the segment, then
// replaying the WAL on top of it.
//
// Checkpointing writes the whole in-memory state out as a fresh segment
// and then clears the WAL, since the segment now covers everything the
// log was holding.
class Database {
public:
    // dir is a folder; it holds lattice.seg and lattice.wal.
    explicit Database(const std::string& dir);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void insert(const Vector& v);
    std::optional<Vector> get(uint64_t id) const;
    size_t size() const;

    void checkpoint();

    const std::string& segment_path() const { return seg_path_; }
    const std::string& wal_path() const { return wal_path_; }

private:
    void recover();

    std::string dir_;
    std::string seg_path_;
    std::string wal_path_;

    FlatStore store_;
    std::unique_ptr<Wal> wal_;
};

}  // namespace lattice
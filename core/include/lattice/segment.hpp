#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "lattice/vector.hpp"

namespace lattice {

// Segment file layout:
//
//   Header (24 bytes):
//     [4]  magic "LTSG"
//     [4]  version   (uint32)
//     [8]  count     (uint64)  number of records that follow
//     [8]  reserved  (zeros)
//
//   Then `count` records, each:
//     [8]  id   (uint64)
//     [4]  dim  (uint32)
//     [dim * 4] float32 data
//
// The header size is fixed at 24 bytes on purpose. Adding a field later
// means eating into the reserved block, not shifting every record offset.

constexpr char kSegmentMagic[4] = {'L', 'T', 'S', 'G'};
constexpr uint32_t kSegmentVersion = 1;
constexpr size_t kSegmentHeaderSize = 24;

// Writes a batch of vectors out to a segment file in one go.
// Writes to <path>.tmp first and renames on success, so a crash halfway
// through never leaves a half-written segment where a good one used to be.
class SegmentWriter {
public:
    static void write(const std::string& path, const std::vector<Vector>& items);
};

// Reads a segment file by mapping it into memory rather than read()ing it.
// The OS pages it in on demand, so opening a large segment is cheap and
// only the parts actually touched cost anything.
class SegmentReader {
public:
    explicit SegmentReader(const std::string& path);
    ~SegmentReader();

    SegmentReader(const SegmentReader&) = delete;
    SegmentReader& operator=(const SegmentReader&) = delete;

    bool ok() const { return data_ != nullptr; }
    uint64_t count() const { return count_; }

    // Materializes every record into normal owned Vectors.
    // Fine for now; a real query path would read straight off the mapping.
    std::vector<Vector> read_all() const;

private:
    int fd_ = -1;
    const unsigned char* data_ = nullptr;
    size_t size_ = 0;
    uint64_t count_ = 0;
};

}  // namespace lattice
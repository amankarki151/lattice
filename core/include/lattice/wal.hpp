#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "lattice/vector.hpp"

namespace lattice {

// Append-only write-ahead log.
//
// On-disk record layout:
//   [uint64 id][uint32 dim][dim * float32 data]
//
// No delimiters. Once you've read the dim you know exactly how many bytes
// the body is. A record that's cut short (process died mid-write) fails the
// read and replay stops there — everything before it is still good.
class Wal {
public:
    explicit Wal(const std::string& path);
    ~Wal();

    // No copying — this owns a file handle.
    Wal(const Wal&) = delete;
    Wal& operator=(const Wal&) = delete;

    void append(const Vector& v);
    void flush();

    // Reads the whole log from the start. Stops at the first bad/short
    // record and returns everything up to that point.
    static std::vector<Vector> replay(const std::string& path);

private:
    std::string path_;
    std::ofstream out_;
};

}  // namespace lattice
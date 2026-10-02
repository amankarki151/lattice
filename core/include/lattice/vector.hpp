#pragma once

#include <cstdint>
#include <vector>

namespace lattice {

// A single stored vector. An id, its floats, and an optional opaque
// payload — arbitrary bytes the caller attaches and gets back untouched.
// Lattice doesn't interpret payload at all; a caller like RAAG serializes
// its own metadata (path, coupling metrics, chunk text) into it however
// it wants.
struct Vector {
    uint64_t id = 0;
    std::vector<float> data;
    std::vector<uint8_t> payload;  // empty by default, fully optional

    uint32_t dim() const {
        return static_cast<uint32_t>(data.size());
    }
};

}  // namespace lattice
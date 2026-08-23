#pragma once

#include <cstdint>
#include <vector>

namespace lattice {

// A single stored vector. Nothing clever here — an id and its floats.
// This will almost certainly change once quantization lands, and that's fine.
struct Vector {
    uint64_t id = 0;
    std::vector<float> data;

    uint32_t dim() const {
        return static_cast<uint32_t>(data.size());
    }
};

}  // namespace lattice
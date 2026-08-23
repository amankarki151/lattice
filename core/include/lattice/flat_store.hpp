#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "lattice/vector.hpp"

namespace lattice {

// The dumbest possible store: everything in a hash map, everything in RAM.
// It exists so the WAL has something to replay into, and so brute-force
// search on Day 3 has something to scan. It gets replaced later.
class FlatStore {
public:
    void insert(const Vector& v);
    std::optional<Vector> get(uint64_t id) const;
    size_t size() const;
    void clear();

private:
    std::unordered_map<uint64_t, Vector> items_;
};

}  // namespace lattice
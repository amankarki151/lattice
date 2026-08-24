#pragma once

#include <cstddef>
#include <vector>

namespace lattice {

// Squared L2 distance. No sqrt on purpose.
//
// sqrt is monotonic, so leaving it out doesn't change the ordering of
// results at all - it just saves a sqrt per comparison. If an actual
// distance value ever needs reporting to a user, take the sqrt then.
//
// Assumes both vectors are the same length. The caller checks that.
float l2_squared(const std::vector<float>& a, const std::vector<float>& b);

}  // namespace lattice
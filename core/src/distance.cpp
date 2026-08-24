#include "lattice/distance.hpp"

namespace lattice {

float l2_squared(const std::vector<float>& a, const std::vector<float>& b) {
    const size_t n = a.size();
    float sum = 0.0f;

    // Plain loop. The compiler auto-vectorizes this at -O2 and above,
    // which is good enough until there's a benchmark saying otherwise.
    for (size_t i = 0; i < n; ++i) {
        const float d = a[i] - b[i];
        sum += d * d;
    }

    return sum;
}

}  // namespace lattice
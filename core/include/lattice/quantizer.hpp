#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "lattice/vector.hpp"

namespace lattice {

// Scalar quantizer: float32 -> uint8, one byte per dimension.
//
// Each dimension gets its own min/max, so a dimension spanning 0.1-0.2
// still uses all 256 buckets instead of being crushed into two or three
// by some other dimension's much wider range.
//
//   scale  = (max - min) / 255
//   encode = round((x - min) / scale)
//   decode = min + q * scale
//
// Max error per dimension is scale/2 - you're rounding to the nearest of
// 256 evenly spaced points.
//
// This has to be trained on real data before it can encode anything.
// Values arriving later that fall outside the trained range get clamped,
// which is why the training sample needs to actually be representative.
class ScalarQuantizer {
public:
    ScalarQuantizer() = default;

    // Works out per-dimension min/max from a sample. Must be called
    // before encode/decode.
    void train(const std::vector<Vector>& sample);

    bool trained() const { return trained_; }
    size_t dim() const { return dim_; }

    std::vector<uint8_t> encode(const std::vector<float>& v) const;
    std::vector<float> decode(const std::vector<uint8_t>& q) const;

    // Distance between an already-encoded vector and a raw float query,
    // without decoding the whole thing into a temporary vector first.
    float distance_to_query(const std::vector<uint8_t>& q,
                            const std::vector<float>& query) const;

    const std::vector<float>& mins() const { return min_; }
    const std::vector<float>& scales() const { return scale_; }

private:
    bool trained_ = false;
    size_t dim_ = 0;
    std::vector<float> min_;
    std::vector<float> scale_;
};

}  // namespace lattice
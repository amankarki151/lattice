#include "lattice/quantizer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lattice {

void ScalarQuantizer::train(const std::vector<Vector>& sample) {
    if (sample.empty()) {
        throw std::runtime_error("quantizer: cannot train on an empty sample");
    }

    dim_ = sample.front().data.size();
    min_.assign(dim_, std::numeric_limits<float>::max());
    scale_.assign(dim_, 0.0f);

    std::vector<float> max_(dim_, std::numeric_limits<float>::lowest());

    for (const auto& v : sample) {
        if (v.data.size() != dim_) continue;  // skip mismatched dims
        for (size_t i = 0; i < dim_; ++i) {
            min_[i] = std::min(min_[i], v.data[i]);
            max_[i] = std::max(max_[i], v.data[i]);
        }
    }

    for (size_t i = 0; i < dim_; ++i) {
        const float range = max_[i] - min_[i];

        // A dimension where every value is identical has zero range.
        // Scale of 0 would mean dividing by zero on encode, so use 1.0 -
        // everything in that dimension encodes to bucket 0 and decodes
        // back to min, which is exactly right since min == max there.
        scale_[i] = (range > 0.0f) ? (range / 255.0f) : 1.0f;
    }

    trained_ = true;
}

std::vector<uint8_t> ScalarQuantizer::encode(
    const std::vector<float>& v) const {
    if (!trained_) {
        throw std::runtime_error("quantizer: encode before train");
    }

    std::vector<uint8_t> out(dim_, 0);

    for (size_t i = 0; i < dim_ && i < v.size(); ++i) {
        float q = (v[i] - min_[i]) / scale_[i];

        // Clamp. A value outside the trained range would otherwise wrap
        // around when cast to uint8 - 256 becomes 0, which is not just
        // inaccurate, it's the opposite end of the range.
        q = std::round(q);
        if (q < 0.0f) q = 0.0f;
        if (q > 255.0f) q = 255.0f;

        out[i] = static_cast<uint8_t>(q);
    }

    return out;
}

std::vector<float> ScalarQuantizer::decode(
    const std::vector<uint8_t>& q) const {
    if (!trained_) {
        throw std::runtime_error("quantizer: decode before train");
    }

    std::vector<float> out(dim_, 0.0f);
    for (size_t i = 0; i < dim_ && i < q.size(); ++i) {
        out[i] = min_[i] + static_cast<float>(q[i]) * scale_[i];
    }
    return out;
}

float ScalarQuantizer::distance_to_query(
    const std::vector<uint8_t>& q, const std::vector<float>& query) const {
    // Decodes one dimension at a time inside the loop rather than building
    // a whole temporary float vector. Same arithmetic, no allocation.
    float sum = 0.0f;
    const size_t n = std::min(dim_, query.size());

    for (size_t i = 0; i < n; ++i) {
        const float decoded = min_[i] + static_cast<float>(q[i]) * scale_[i];
        const float d = decoded - query[i];
        sum += d * d;
    }

    return sum;
}

}  // namespace lattice
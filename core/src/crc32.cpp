#include "lattice/crc32.hpp"

namespace lattice {

uint32_t crc32(const void* data, size_t len) {
    static const uint32_t poly = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;

    const auto* bytes = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < len; ++i) {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            const uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (poly & mask);
        }
    }

    return ~crc;
}

}  // namespace lattice
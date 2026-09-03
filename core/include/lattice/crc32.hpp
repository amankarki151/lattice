#pragma once

#include <cstddef>
#include <cstdint>

namespace lattice {

// Standard CRC32 (IEEE 802.3 polynomial), byte-by-byte, no table
// lookup or hardware instruction. Not the fastest possible
// implementation, but simple, correct, and fast enough for
// detecting accidental bit-flip corruption. This is an integrity
// check, not a security checksum - it won't catch a deliberate,
// crafted tampering attempt.
uint32_t crc32(const void* data, size_t len);

}  // namespace lattice
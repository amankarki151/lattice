#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "lattice/crc32.hpp"

namespace {

TEST(Crc32Test, EmptyInputIsZero) {
    EXPECT_EQ(lattice::crc32(nullptr, 0), 0u);
}

// Standard CRC32 test vector - "123456789" is the canonical string
// used to verify CRC32 implementations against the known-correct
// result. If this fails, the polynomial or bit order is wrong.
TEST(Crc32Test, KnownTestVector) {
    const char* data = "123456789";
    EXPECT_EQ(lattice::crc32(data, 9), 0xCBF43926u);
}

TEST(Crc32Test, DifferentInputsProduceDifferentChecksums) {
    const char* a = "hello world";
    const char* b = "hello worle";  // one character different
    EXPECT_NE(lattice::crc32(a, 11), lattice::crc32(b, 11));
}

TEST(Crc32Test, SameInputAlwaysProducesSameChecksum) {
    const char* a = "deterministic";
    EXPECT_EQ(lattice::crc32(a, 13), lattice::crc32(a, 13));
}

// A single flipped bit anywhere in the input should change the
// checksum. This is the actual property the whole feature depends
// on - if this failed, checksums wouldn't reliably catch corruption.
TEST(Crc32Test, SingleBitFlipChangesChecksum) {
    std::vector<unsigned char> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    const uint32_t original = lattice::crc32(data.data(), data.size());

    for (size_t byte_idx = 0; byte_idx < data.size(); ++byte_idx) {
        for (int bit = 0; bit < 8; ++bit) {
            std::vector<unsigned char> flipped = data;
            flipped[byte_idx] ^= (1u << bit);
            const uint32_t changed =
                lattice::crc32(flipped.data(), flipped.size());
            EXPECT_NE(changed, original)
                << "byte " << byte_idx << " bit " << bit
                << " did not change the checksum";
        }
    }
}

TEST(Crc32Test, LongInputDoesNotCrashOrHang) {
    std::vector<unsigned char> data(100000, 0xAB);
    // Just confirming it runs to completion and returns something
    // deterministic - not asserting a specific value.
    const uint32_t first = lattice::crc32(data.data(), data.size());
    const uint32_t second = lattice::crc32(data.data(), data.size());
    EXPECT_EQ(first, second);
}

}  // namespace
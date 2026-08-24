#include "lattice/segment.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace lattice {

void SegmentWriter::write(const std::string& path,
                          const std::vector<Vector>& items) {
    const std::string tmp = path + ".tmp";

    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("segment: could not open " + tmp);
    }

    // Header.
    const uint32_t version = kSegmentVersion;
    const uint64_t count = items.size();
    const uint64_t reserved = 0;

    out.write(kSegmentMagic, 4);
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    out.write(reinterpret_cast<const char*>(&reserved), sizeof(reserved));

    // Records.
    for (const auto& v : items) {
        const uint64_t id = v.id;
        const uint32_t dim = v.dim();

        out.write(reinterpret_cast<const char*>(&id), sizeof(id));
        out.write(reinterpret_cast<const char*>(&dim), sizeof(dim));
        out.write(reinterpret_cast<const char*>(v.data.data()),
                  static_cast<std::streamsize>(dim) * sizeof(float));
    }

    out.flush();
    out.close();

    // Rename is atomic on the same filesystem. Either the old segment is
    // there or the new one is — never a half-written thing in between.
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        throw std::runtime_error("segment: rename failed for " + path);
    }
}

SegmentReader::SegmentReader(const std::string& path) {
    fd_ = ::open(path.c_str(), O_RDONLY);
    if (fd_ < 0) {
        return;  // no segment yet is not an error
    }

    struct stat st {};
    if (::fstat(fd_, &st) != 0 ||
        static_cast<size_t>(st.st_size) < kSegmentHeaderSize) {
        ::close(fd_);
        fd_ = -1;
        return;
    }
    size_ = static_cast<size_t>(st.st_size);

    void* mapped = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
    if (mapped == MAP_FAILED) {
        ::close(fd_);
        fd_ = -1;
        return;
    }
    data_ = static_cast<const unsigned char*>(mapped);

    // Check the magic before trusting anything else in the file.
    if (std::memcmp(data_, kSegmentMagic, 4) != 0) {
        ::munmap(const_cast<unsigned char*>(data_), size_);
        ::close(fd_);
        data_ = nullptr;
        fd_ = -1;
        return;
    }

    uint32_t version = 0;
    std::memcpy(&version, data_ + 4, sizeof(version));
    if (version != kSegmentVersion) {
        ::munmap(const_cast<unsigned char*>(data_), size_);
        ::close(fd_);
        data_ = nullptr;
        fd_ = -1;
        return;
    }

    std::memcpy(&count_, data_ + 8, sizeof(count_));
}

SegmentReader::~SegmentReader() {
    if (data_ != nullptr) {
        ::munmap(const_cast<unsigned char*>(data_), size_);
    }
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

std::vector<Vector> SegmentReader::read_all() const {
    std::vector<Vector> result;
    if (data_ == nullptr) {
        return result;
    }
    result.reserve(count_);

    size_t offset = kSegmentHeaderSize;

    for (uint64_t i = 0; i < count_; ++i) {
        // Bail out rather than read past the end of the mapping. A truncated
        // file will trip this instead of segfaulting.
        if (offset + sizeof(uint64_t) + sizeof(uint32_t) > size_) break;

        uint64_t id = 0;
        uint32_t dim = 0;

        // memcpy rather than casting the pointer straight to uint64_t*.
        // Record offsets aren't guaranteed to be aligned, and a misaligned
        // load is undefined behaviour even where the hardware tolerates it.
        std::memcpy(&id, data_ + offset, sizeof(id));
        offset += sizeof(id);
        std::memcpy(&dim, data_ + offset, sizeof(dim));
        offset += sizeof(dim);

        if (dim == 0 || dim > 65536) break;

        const size_t bytes = static_cast<size_t>(dim) * sizeof(float);
        if (offset + bytes > size_) break;

        Vector v;
        v.id = id;
        v.data.resize(dim);
        std::memcpy(v.data.data(), data_ + offset, bytes);
        offset += bytes;

        result.push_back(std::move(v));
    }

    return result;
}

}  // namespace lattice
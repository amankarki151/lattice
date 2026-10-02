#include "lattice/wal.hpp"

#include <stdexcept>

namespace lattice {

Wal::Wal(const std::string& path) : path_(path) {
    out_.open(path_, std::ios::binary | std::ios::app);
    if (!out_) {
        throw std::runtime_error("wal: could not open " + path_);
    }
}

Wal::~Wal() {
    if (out_.is_open()) {
        out_.flush();
        out_.close();
    }
}

void Wal::flush() {
    out_.flush();
}

void Wal::append(const Vector& v) {
    const uint64_t id = v.id;
    const uint32_t dim = v.dim();
    const uint32_t payload_len = static_cast<uint32_t>(v.payload.size());

    out_.write(reinterpret_cast<const char*>(&id), sizeof(id));
    out_.write(reinterpret_cast<const char*>(&dim), sizeof(dim));
    out_.write(reinterpret_cast<const char*>(v.data.data()),
               static_cast<std::streamsize>(dim) * sizeof(float));
    out_.write(reinterpret_cast<const char*>(&payload_len), sizeof(payload_len));
    if (payload_len > 0) {
        out_.write(reinterpret_cast<const char*>(v.payload.data()),
                   static_cast<std::streamsize>(payload_len));
    }

    // Flushing on every append is slow and I know it. Correctness first —
    // batching goes in once there's a benchmark to prove it helped.
    out_.flush();
}

std::vector<Vector> Wal::replay(const std::string& path) {
    std::vector<Vector> result;

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return result;  // no log yet is not an error
    }

    while (true) {
        uint64_t id = 0;
        uint32_t dim = 0;

        in.read(reinterpret_cast<char*>(&id), sizeof(id));
        if (in.gcount() != sizeof(id)) break;

        in.read(reinterpret_cast<char*>(&dim), sizeof(dim));
        if (in.gcount() != sizeof(dim)) break;

        // Sanity check. A garbage dim from a torn write would otherwise
        // try to allocate something enormous.
        if (dim == 0 || dim > 65536) break;

        Vector v;
        v.id = id;
        v.data.resize(dim);

        const std::streamsize bytes =
            static_cast<std::streamsize>(dim) * sizeof(float);
        in.read(reinterpret_cast<char*>(v.data.data()), bytes);
        if (in.gcount() != bytes) break;  // torn record, stop here

        uint32_t payload_len = 0;
        in.read(reinterpret_cast<char*>(&payload_len), sizeof(payload_len));
        if (in.gcount() != sizeof(payload_len)) break;

        // same reasoning as the dim check above - a torn write shouldn't
        // make us try to allocate something absurd
        if (payload_len > 16 * 1024 * 1024) break;

        if (payload_len > 0) {
            v.payload.resize(payload_len);
            const std::streamsize payload_bytes =
                static_cast<std::streamsize>(payload_len);
            in.read(reinterpret_cast<char*>(v.payload.data()), payload_bytes);
            if (in.gcount() != payload_bytes) break;  // torn record, stop here
        }

        result.push_back(std::move(v));
    }

    return result;
}

}  // namespace lattice
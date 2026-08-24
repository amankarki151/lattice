#include <iostream>
#include <string>

#include "lattice/segment.hpp"
#include "lattice/vector.hpp"

namespace {

const std::string kSegPath = "/tmp/lattice_test.seg";

std::vector<lattice::Vector> make_vectors(int count) {
    std::vector<lattice::Vector> items;
    items.reserve(count);
    for (int i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = {static_cast<float>(i),
                  static_cast<float>(i) * 2.0f,
                  static_cast<float>(i) * 3.0f,
                  static_cast<float>(i) * 4.0f};
        items.push_back(std::move(v));
    }
    return items;
}

void write_segment(int count) {
    auto items = make_vectors(count);
    lattice::SegmentWriter::write(kSegPath, items);
    std::cout << "wrote segment with " << count << " records\n";
}

void read_segment() {
    lattice::SegmentReader reader(kSegPath);
    if (!reader.ok()) {
        std::cout << "no readable segment at " << kSegPath << "\n";
        return;
    }

    std::cout << "header says " << reader.count() << " records\n";

    auto items = reader.read_all();
    std::cout << "actually read " << items.size() << "\n";

    if (!items.empty()) {
        const auto& first = items.front();
        std::cout << "id " << first.id << " -> [";
        for (size_t i = 0; i < first.data.size(); ++i) {
            std::cout << first.data[i];
            if (i + 1 < first.data.size()) std::cout << ", ";
        }
        std::cout << "]\n";

        const auto& last = items.back();
        std::cout << "last id " << last.id << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: scratch segwrite <count> | scratch segread\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "segwrite") {
        const int count = (argc >= 3) ? std::stoi(argv[2]) : 100;
        write_segment(count);
    } else if (cmd == "segread") {
        read_segment();
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
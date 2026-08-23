#include <iostream>
#include <string>

#include "lattice/flat_store.hpp"
#include "lattice/wal.hpp"

namespace {

const std::string kWalPath = "/tmp/lattice_test.wal";

void write_records(int count) {
    lattice::Wal wal(kWalPath);
    for (int i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = {static_cast<float>(i),
                  static_cast<float>(i) * 2.0f,
                  static_cast<float>(i) * 3.0f,
                  static_cast<float>(i) * 4.0f};
        wal.append(v);

        if (i % 1000 == 0) {
            std::cout << "wrote " << i << "\n";
        }
    }
    std::cout << "done writing " << count << "\n";
}

void replay_into_store() {
    auto records = lattice::Wal::replay(kWalPath);

    lattice::FlatStore store;
    for (const auto& v : records) {
        store.insert(v);
    }

    std::cout << "replayed " << records.size() << " records, store has "
              << store.size() << "\n";

    auto first = store.get(1);
    if (first) {
        std::cout << "id 1 -> [";
        for (size_t i = 0; i < first->data.size(); ++i) {
            std::cout << first->data[i];
            if (i + 1 < first->data.size()) std::cout << ", ";
        }
        std::cout << "]\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: scratch write <count> | scratch replay\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "write") {
        const int count = (argc >= 3) ? std::stoi(argv[2]) : 100;
        write_records(count);
    } else if (cmd == "replay") {
        replay_into_store();
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
#include <iostream>
#include <string>

#include "lattice/database.hpp"

namespace {

const std::string kDir = "/tmp/lattice_db";

lattice::Vector make_vector(uint64_t id) {
    lattice::Vector v;
    v.id = id;
    v.data = {static_cast<float>(id),
              static_cast<float>(id) * 2.0f,
              static_cast<float>(id) * 3.0f,
              static_cast<float>(id) * 4.0f};
    return v;
}

void cmd_insert(uint64_t from, uint64_t to) {
    lattice::Database db(kDir);
    for (uint64_t i = from; i <= to; ++i) {
        db.insert(make_vector(i));
        if (i % 1000 == 0) {
            std::cout << "inserted up to " << i << "\n";
        }
    }
    std::cout << "done. store size " << db.size() << "\n";
}

void cmd_checkpoint() {
    lattice::Database db(kDir);
    db.checkpoint();
}

void cmd_open() {
    lattice::Database db(kDir);
    std::cout << "opened, size " << db.size() << "\n";

    auto first = db.get(1);
    std::cout << "id 1 present? " << (first ? "yes" : "no") << "\n";
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
        std::cout << "usage:\n"
                  << "  scratch insert <from> <to>\n"
                  << "  scratch checkpoint\n"
                  << "  scratch open\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "insert") {
        if (argc < 4) {
            std::cout << "insert needs <from> <to>\n";
            return 1;
        }
        cmd_insert(std::stoull(argv[2]), std::stoull(argv[3]));
    } else if (cmd == "checkpoint") {
        cmd_checkpoint();
    } else if (cmd == "open") {
        cmd_open();
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
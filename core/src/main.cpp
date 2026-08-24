#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "lattice/database.hpp"

namespace {

const std::string kDir = "/tmp/lattice_db";
constexpr size_t kDim = 8;

// Deterministic on purpose - same seed means the same vectors every run,
// so a result that looks wrong can actually be re-checked.
std::vector<float> random_vector(std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::vector<float> v(kDim);
    for (size_t i = 0; i < kDim; ++i) {
        v[i] = dist(rng);
    }
    return v;
}

void cmd_fill(size_t count) {
    lattice::Database db(kDir);
    std::mt19937 rng(42);

    for (size_t i = 1; i <= count; ++i) {
        lattice::Vector v;
        v.id = static_cast<uint64_t>(i);
        v.data = random_vector(rng);
        db.insert(v);
    }

    std::cout << "filled, store size " << db.size() << "\n";
}

void print_results(const std::vector<lattice::SearchResult>& hits) {
    for (size_t i = 0; i < hits.size(); ++i) {
        std::cout << "  " << (i + 1) << ". id " << hits[i].id
                  << "  dist " << hits[i].distance << "\n";
    }
}

void cmd_search(uint64_t query_id, size_t k) {
    lattice::Database db(kDir);

    auto target = db.get(query_id);
    if (!target) {
        std::cout << "no vector with id " << query_id << "\n";
        return;
    }

    auto hits = db.search(target->data, k);
    std::cout << "top " << k << " for id " << query_id << ":\n";
    print_results(hits);
}

void cmd_searchrand(size_t k) {
    lattice::Database db(kDir);

    std::mt19937 rng(999);  // different seed - not one of the stored vectors
    auto query = random_vector(rng);

    auto hits = db.search(query, k);
    std::cout << "top " << k << " for a random query:\n";
    print_results(hits);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage:\n"
                  << "  scratch fill <count>\n"
                  << "  scratch search <id> <k>\n"
                  << "  scratch searchrand <k>\n"
                  << "  scratch checkpoint\n"
                  << "  scratch open\n";
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "fill") {
        cmd_fill(argc >= 3 ? std::stoull(argv[2]) : 1000);
    } else if (cmd == "search") {
        if (argc < 4) {
            std::cout << "search needs <id> <k>\n";
            return 1;
        }
        cmd_search(std::stoull(argv[2]), std::stoull(argv[3]));
    } else if (cmd == "searchrand") {
        cmd_searchrand(argc >= 3 ? std::stoull(argv[2]) : 5);
    } else if (cmd == "checkpoint") {
        lattice::Database db(kDir);
        db.checkpoint();
    } else if (cmd == "open") {
        lattice::Database db(kDir);
        std::cout << "opened, size " << db.size() << "\n";
    } else {
        std::cout << "unknown command: " << cmd << "\n";
        return 1;
    }

    return 0;
}
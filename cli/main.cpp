#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lattice/database.hpp"

namespace {

void print_usage() {
    std::cout
        << "lattice - embedded vector database\n\n"
        << "usage:\n"
        << "  lattice <dir> insert <id> <v1,v2,...>\n"
        << "  lattice <dir> get <id>\n"
        << "  lattice <dir> query <v1,v2,...> <k>\n"
        << "  lattice <dir> checkpoint\n"
        << "  lattice <dir> stats\n";
}

// Parses "1.0,2.5,3.0" into floats.
std::vector<float> parse_vector(const std::string& s) {
    std::vector<float> out;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, ',')) {
        if (part.empty()) continue;
        out.push_back(std::stof(part));
    }
    return out;
}

void print_vector(const std::vector<float>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    const std::string dir = argv[1];
    const std::string cmd = argv[2];

    lattice::Database db(dir);

    if (cmd == "insert") {
        if (argc < 5) {
            std::cout << "insert needs <id> <v1,v2,...>\n";
            return 1;
        }
        lattice::Vector v;
        v.id = std::stoull(argv[3]);
        v.data = parse_vector(argv[4]);
        if (v.data.empty()) {
            std::cout << "could not parse any floats from: " << argv[4] << "\n";
            return 1;
        }
        db.insert(v);
        std::cout << "inserted id " << v.id << " dim " << v.dim() << "\n";

    } else if (cmd == "get") {
        if (argc < 4) {
            std::cout << "get needs <id>\n";
            return 1;
        }
        auto v = db.get(std::stoull(argv[3]));
        if (!v) {
            std::cout << "not found\n";
            return 1;
        }
        std::cout << "id " << v->id << " -> ";
        print_vector(v->data);
        std::cout << "\n";

    } else if (cmd == "query") {
        if (argc < 5) {
            std::cout << "query needs <v1,v2,...> <k>\n";
            return 1;
        }
        auto q = parse_vector(argv[3]);
        const size_t k = std::stoull(argv[4]);
        auto hits = db.search(q, k);
        if (hits.empty()) {
            std::cout << "no results\n";
            return 0;
        }
        for (size_t i = 0; i < hits.size(); ++i) {
            std::cout << (i + 1) << ". id " << hits[i].id
                      << "  dist " << hits[i].distance << "\n";
        }

    } else if (cmd == "checkpoint") {
        db.checkpoint();

    } else if (cmd == "stats") {
        std::cout << "vectors: " << db.size() << "\n"
                  << "segment: " << db.segment_path() << "\n"
                  << "wal:     " << db.wal_path() << "\n";

    } else {
        std::cout << "unknown command: " << cmd << "\n\n";
        print_usage();
        return 1;
    }

    return 0;
}
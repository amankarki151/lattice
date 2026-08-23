#include <iostream>

#include "lattice/flat_store.hpp"

int main() {
    lattice::FlatStore store;

    for (uint64_t i = 1; i <= 3; ++i) {
        lattice::Vector v;
        v.id = i;
        v.data = {static_cast<float>(i) * 1.0f,
                  static_cast<float>(i) * 2.0f,
                  static_cast<float>(i) * 3.0f};
        store.insert(v);
    }

    std::cout << "stored " << store.size() << " vectors\n";

    auto found = store.get(2);
    if (found) {
        std::cout << "id 2 -> [";
        for (size_t i = 0; i < found->data.size(); ++i) {
            std::cout << found->data[i];
            if (i + 1 < found->data.size()) std::cout << ", ";
        }
        std::cout << "]\n";
    } else {
        std::cout << "id 2 not found\n";
    }

    auto missing = store.get(99);
    std::cout << "id 99 found? " << (missing ? "yes" : "no") << "\n";

    return 0;
}
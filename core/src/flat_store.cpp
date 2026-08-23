#include "lattice/flat_store.hpp"

namespace lattice {

void FlatStore::insert(const Vector& v) {
    items_[v.id] = v;
}

std::optional<Vector> FlatStore::get(uint64_t id) const {
    auto it = items_.find(id);
    if (it == items_.end()) {
        return std::nullopt;
    }
    return it->second;
}

size_t FlatStore::size() const {
    return items_.size();
}

void FlatStore::clear() {
    items_.clear();
}

}  // namespace lattice
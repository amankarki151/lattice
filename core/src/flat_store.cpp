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

std::vector<Vector> FlatStore::snapshot() const {
    std::vector<Vector> out;
    out.reserve(items_.size());
    for (const auto& [id, v] : items_) {
        out.push_back(v);
    }
    return out;
}

}  // namespace lattice
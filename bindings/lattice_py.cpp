#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lattice/concurrent_index.hpp"
#include "lattice/database.hpp"
#include "lattice/hnsw.hpp"
#include "lattice/search.hpp"
#include "lattice/vector.hpp"

namespace py = pybind11;

// What gets exposed here is the database's interface, not its internals.
// WAL, SegmentReader, and the guts of HNSW stay on the C++ side - they're
// implementation details, and exposing them would lock in decisions that
// should stay free to change.
//
// Note the call_guard<gil_scoped_release> on search and insert. Python
// holds a global interpreter lock, and a long C++ call that keeps holding
// it blocks every other Python thread. Releasing it means the concurrency
// work from Day 7 actually pays off from Python instead of being
// serialized behind the GIL. It's safe here because the C++ side does its
// own locking and never touches Python objects while running.

PYBIND11_MODULE(lattice, m) {
    m.doc() = "Lattice - an embedded vector database";

    py::class_<lattice::Vector>(m, "Vector")
        .def(py::init<>())
        .def(py::init([](uint64_t id, std::vector<float> data) {
                 lattice::Vector v;
                 v.id = id;
                 v.data = std::move(data);
                 return v;
             }),
             py::arg("id"), py::arg("data"))
        .def_readwrite("id", &lattice::Vector::id)
        .def_readwrite("data", &lattice::Vector::data)
        .def("dim", &lattice::Vector::dim)
        .def("__repr__", [](const lattice::Vector& v) {
            return "<Vector id=" + std::to_string(v.id) +
                   " dim=" + std::to_string(v.dim()) + ">";
        });

    py::class_<lattice::SearchResult>(m, "SearchResult")
        .def_readonly("id", &lattice::SearchResult::id)
        .def_readonly("distance", &lattice::SearchResult::distance)
        .def("__repr__", [](const lattice::SearchResult& r) {
            return "<SearchResult id=" + std::to_string(r.id) +
                   " distance=" + std::to_string(r.distance) + ">";
        });

    py::class_<lattice::HnswConfig>(m, "HnswConfig")
        .def(py::init<>())
        .def_readwrite("M", &lattice::HnswConfig::M)
        .def_readwrite("M_max0", &lattice::HnswConfig::M_max0)
        .def_readwrite("ef_construction",
                       &lattice::HnswConfig::ef_construction)
        .def_readwrite("seed", &lattice::HnswConfig::seed);

    py::class_<lattice::Database>(m, "Database")
        .def(py::init<const std::string&>(), py::arg("directory"))
        .def("insert", &lattice::Database::insert, py::arg("vector"),
             py::call_guard<py::gil_scoped_release>())
        .def("get", &lattice::Database::get, py::arg("id"))
        .def("search", &lattice::Database::search, py::arg("query"),
             py::arg("k"), py::call_guard<py::gil_scoped_release>())
        .def("checkpoint", &lattice::Database::checkpoint,
             py::call_guard<py::gil_scoped_release>())
        .def("size", &lattice::Database::size)
        .def("__len__", &lattice::Database::size);

    py::class_<lattice::ConcurrentIndex>(m, "ConcurrentIndex")
        .def(py::init<lattice::HnswConfig>(),
             py::arg("config") = lattice::HnswConfig{})
        .def("insert", &lattice::ConcurrentIndex::insert, py::arg("vector"),
             py::call_guard<py::gil_scoped_release>())
        .def("search", &lattice::ConcurrentIndex::search, py::arg("query"),
             py::arg("k"), py::arg("ef") = 50,
             py::call_guard<py::gil_scoped_release>())
        .def("size", &lattice::ConcurrentIndex::size)
        .def("max_layer", &lattice::ConcurrentIndex::max_layer)
        .def("insert_count", &lattice::ConcurrentIndex::insert_count)
                .def("search_count", &lattice::ConcurrentIndex::search_count)
        .def("__len__", &lattice::ConcurrentIndex::size)
        .def("insert_batch", &lattice::ConcurrentIndex::insert_batch,
             py::arg("vectors"),
             py::call_guard<py::gil_scoped_release>());
        }
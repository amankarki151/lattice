find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(lattice_py lattice_py.cpp)

target_link_libraries(lattice_py PRIVATE lattice_core)

# The Python module has to be importable as "lattice", but the CMake
# target can't also be called lattice - the CLI executable already has
# that name. So the target is lattice_py and the output file gets
# renamed to lattice.
set_target_properties(lattice_py PROPERTIES OUTPUT_NAME lattice)
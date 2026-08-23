add_library(lattice_core
    src/flat_store.cpp
    src/wal.cpp
)

target_include_directories(lattice_core PUBLIC include)

add_executable(scratch src/main.cpp)
target_link_libraries(scratch PRIVATE lattice_core)
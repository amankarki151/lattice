add_library(lattice_core
    src/flat_store.cpp
    src/wal.cpp
    src/segment.cpp
    src/database.cpp
    src/distance.cpp
    src/search.cpp
    src/hnsw.cpp
    src/quantizer.cpp
    src/concurrent_index.cpp
)

target_include_directories(lattice_core PUBLIC include)

find_package(Threads REQUIRED)
target_link_libraries(lattice_core PUBLIC Threads::Threads)

add_executable(scratch src/main.cpp)
target_link_libraries(scratch PRIVATE lattice_core)
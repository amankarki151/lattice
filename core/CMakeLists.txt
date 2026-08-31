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

option(LATTICE_BUILD_TESTS "Build the test suite" OFF)
if(LATTICE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(LATTICE_COVERAGE "Build with coverage instrumentation" OFF)
if(LATTICE_COVERAGE)
    target_compile_options(lattice_core PUBLIC --coverage -O0 -g)
    target_link_options(lattice_core PUBLIC --coverage)
endif()
cmake_minimum_required(VERSION 3.14)
project(Lattice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lattice_core
    core/src/flat_store.cpp
    core/src/wal.cpp
    core/src/segment.cpp
    core/src/database.cpp
    core/src/distance.cpp
    core/src/search.cpp
    core/src/hnsw.cpp
    core/src/quantizer.cpp
    core/src/concurrent_index.cpp
)

# Assuming your include folder is also inside core. 
# If it is in the root directory, leave this as 'PUBLIC include'
target_include_directories(lattice_core PUBLIC core/include)

find_package(Threads REQUIRED)
target_link_libraries(lattice_core PUBLIC Threads::Threads)

# Also updated the path for main.cpp
add_executable(scratch core/src/main.cpp)
target_link_libraries(scratch PRIVATE lattice_core)
set_target_properties(scratch PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/core")


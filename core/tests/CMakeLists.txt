include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# Stops gtest from overriding the parent project's compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lattice_tests
    test_wal.cpp
    test_segment.cpp
    test_search.cpp
    test_hnsw.cpp
    test_quantizer.cpp
    test_crc32.cpp
)

target_link_libraries(lattice_tests PRIVATE lattice_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lattice_tests)


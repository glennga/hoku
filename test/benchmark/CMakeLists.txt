##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestBenchmark test-benchmark.cpp)
target_link_libraries(TestBenchmark BaseTest Benchmark Rotation
        Nibble Star SQLiteCpp Sqlite3)

##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestBenchmark
##################################################################
add_executable(TestBenchmark test-benchmark.cpp)
target_link_libraries(TestBenchmark BaseTest Benchmark Rotation
        Nibble Star SQLiteCpp Sqlite3)

##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/trio)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/chomp)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)
include_directories(${CMAKE_SOURCE_DIR}/lib/identification/planar-triangle)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestPlanarTriangle test-planar-triangle.cpp)
target_link_libraries(TestPlanarTriangle BaseTest PlanarTriangle
        Benchmark Rotation Chomp Nibble Trio Star SQLiteCpp Sqlite3)

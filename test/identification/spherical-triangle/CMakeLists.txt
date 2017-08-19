##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/trio)
include_directories(${CMAKE_SOURCE_DIR}/lib/tree/quad-node)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/mercator)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/chomp)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)
include_directories(${CMAKE_SOURCE_DIR}/lib/identification/base-triangle)
include_directories(${CMAKE_SOURCE_DIR}/lib/identification/spherical-triangle)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestSphericalTriangle test-spherical-triangle.cpp)
target_link_libraries(TestSphericalTriangle BaseTest SphericalTriangle
        BaseTriangle Benchmark Rotation QuadNode Chomp Nibble Mercator Trio
        Star SQLiteCpp Sqlite3)

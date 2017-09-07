##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/mercator)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/asterism)
include_directories(${CMAKE_SOURCE_DIR}/lib/tree/kd-node)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/chomp)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)
include_directories(${CMAKE_SOURCE_DIR}/lib/identification/astrometry-net)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestAstrometryNet test-astrometry-net.cpp)
target_link_libraries(TestAstrometryNet BaseTest AstrometryNet
        Benchmark Rotation KdNode Chomp Nibble Asterism Mercator
        Star SQLiteCpp Sqlite3)

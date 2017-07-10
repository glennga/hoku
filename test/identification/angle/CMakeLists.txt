##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base_test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)
include_directories(${CMAKE_SOURCE_DIR}/lib/identification/angle)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestAngle test_angle.cpp)
target_link_libraries(TestAngle BaseTest Angle Benchmark Rotation
        Nibble Star SQLiteCpp Sqlite3)


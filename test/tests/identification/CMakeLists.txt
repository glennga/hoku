##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestAngle
##################################################################
add_executable(TestAngle test-angle.cpp)
target_link_libraries(TestAngle gtest gmock Angle Identification
        Benchmark Rotation Chomp Nibble Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestPlanarTriangle
##################################################################
add_executable(TestPlanarTriangle test-planar-triangle.cpp)
target_link_libraries(TestPlanarTriangle gtest gmock PlanarTriangle
        BaseTriangle Identification Benchmark Rotation Chomp Nibble
        Trio Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestSphericalTriangle
##################################################################
add_executable(TestSphericalTriangle test-spherical-triangle.cpp)
target_link_libraries(TestSphericalTriangle gtest gmock SphericalTriangle
        BaseTriangle Identification Benchmark Rotation Chomp Nibble
        Trio Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestPyramid
##################################################################
add_executable(TestPyramid test-pyramid.cpp)
target_link_libraries(TestPyramid gtest Pyramid Angle Identification
        Chomp Benchmark Rotation Nibble Star SQLiteCpp Sqlite3)

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
target_link_libraries(TestAngle BaseTest Angle Benchmark Rotation
        Chomp Nibble Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestPlanarTriangle
##################################################################
add_executable(TestPlanarTriangle test-planar-triangle.cpp)
target_link_libraries(TestPlanarTriangle BaseTest PlanarTriangle
        BaseTriangle Benchmark Rotation Chomp Nibble Trio
        Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestSphericalTriangle
##################################################################
add_executable(TestSphericalTriangle test-spherical-triangle.cpp)
target_link_libraries(TestSphericalTriangle BaseTest SphericalTriangle
        BaseTriangle Benchmark Rotation Chomp Nibble Trio
        Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestPyramid
##################################################################
add_executable(TestPyramid test-pyramid.cpp)
target_link_libraries(TestPyramid BaseTest Pyramid Angle Chomp
        Benchmark Rotation Nibble Star SQLiteCpp Sqlite3)

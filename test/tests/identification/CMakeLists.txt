##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestIdentification
##################################################################
add_executable(TestIdentification test-identification.cpp)
target_link_libraries(TestIdentification gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestAngle
##################################################################
add_executable(TestAngle test-angle.cpp)
target_link_libraries(TestAngle gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestPlanarTriangle
##################################################################
add_executable(TestPlanarTriangle test-planar-triangle.cpp)
target_link_libraries(TestPlanarTriangle gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestSphericalTriangle
##################################################################
add_executable(TestSphericalTriangle test-spherical-triangle.cpp)
target_link_libraries(TestSphericalTriangle gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestPyramid
##################################################################
add_executable(TestPyramid test-pyramid.cpp)
target_link_libraries(TestPyramid gtest gmock ${HOKU_LIBS})

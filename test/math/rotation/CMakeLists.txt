##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestRotation test-rotation.cpp)
target_link_libraries(TestRotation BaseTest Star Rotation)

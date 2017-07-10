##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/trio)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestTrio test_trio.cpp)
target_link_libraries(TestTrio BaseTest Star Trio)

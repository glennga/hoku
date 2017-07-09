##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/base_test)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestStar test_star.cpp)
target_link_libraries(TestStar BaseTest Star)
##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestStar test_star.cpp)
target_link_libraries(TestStar BaseTest Star)

##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestStar test-star.cpp)
target_link_libraries(TestStar BaseTest Star)
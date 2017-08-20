##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/mercator)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/asterism)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestAsterism test-asterism.cpp)
target_link_libraries(TestAsterism BaseTest Asterism Mercator Star)

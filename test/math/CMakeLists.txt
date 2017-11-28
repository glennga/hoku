##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestStar
##################################################################
add_executable(TestStar test-star.cpp)
target_link_libraries(TestStar BaseTest Star)

##################################################################
# Link and Create Executable TestRotation
##################################################################
add_executable(TestRotation test-rotation.cpp)
target_link_libraries(TestRotation BaseTest Star Rotation)

##################################################################
# Link and Create Executable TestAsterism
##################################################################
add_executable(TestAsterism test-asterism.cpp)
target_link_libraries(TestAsterism BaseTest Asterism Mercator Star)

##################################################################
# Link and Create Executable TestMercator
##################################################################
add_executable(TestMercator test-mercator.cpp)
target_link_libraries(TestMercator BaseTest Mercator Star)

##################################################################
# Link and Create Executable TestTrio
##################################################################
add_executable(TestTrio test-trio.cpp)
target_link_libraries(TestTrio BaseTest Rotation Star Trio)
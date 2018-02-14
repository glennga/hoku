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
target_link_libraries(TestStar gtest ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestRotation
##################################################################
add_executable(TestRotation test-rotation.cpp)
target_link_libraries(TestRotation gtest ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestAsterism
##################################################################
add_executable(TestAsterism test-asterism.cpp)
target_link_libraries(TestAsterism gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestMercator
##################################################################
add_executable(TestMercator test-mercator.cpp)
target_link_libraries(TestMercator gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestTrio
##################################################################
add_executable(TestTrio test-trio.cpp)
target_link_libraries(TestTrio gtest ${HOKU_LIBS})

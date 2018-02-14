##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestNibble
##################################################################
add_executable(TestNibble test-nibble.cpp)
target_link_libraries(TestNibble gtest ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestChomp
##################################################################
add_executable(TestChomp test-chomp.cpp)
target_link_libraries(TestChomp gtest gmock ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestKdNode
##################################################################
add_executable(TestKdNode test-kd-node.cpp)
target_link_libraries(TestKdNode gtest ${HOKU_LIBS})

##################################################################
# Link and Create Executable TestQuadNode
##################################################################
add_executable(TestQuadNode test-quad-node.cpp)
target_link_libraries(TestQuadNode gtest ${HOKU_LIBS})


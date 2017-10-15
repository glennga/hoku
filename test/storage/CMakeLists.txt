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
target_link_libraries(TestNibble BaseTest Chomp Nibble Star SQLiteCpp
        Sqlite3)

##################################################################
# Link and Create Executable TestChomp
##################################################################
add_executable(TestChomp test-chomp.cpp)
target_link_libraries(TestChomp BaseTest Chomp Nibble Star
        SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestKdNode
##################################################################
add_executable(TestKdNode test-kd-node.cpp)
target_link_libraries(TestKdNode BaseTest KdNode Nibble Mercator
        Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TestQuadNode
##################################################################
add_executable(TestQuadNode test-quad-node.cpp)
target_link_libraries(TestQuadNode BaseTest QuadNode Nibble Mercator
        Star SQLiteCpp Sqlite3)


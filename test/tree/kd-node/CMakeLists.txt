##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/mercator)
include_directories(${CMAKE_SOURCE_DIR}/lib/tree/kd-node)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestKdNode test-kd-node.cpp)
target_link_libraries(TestKdNode BaseTest KdNode Nibble Mercator
        Star SQLiteCpp Sqlite3)

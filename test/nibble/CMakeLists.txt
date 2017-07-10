##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base_test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/nibble)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestNibble test_nibble.cpp)
target_link_libraries(TestNibble BaseTest Nibble Star SQLiteCpp
        Sqlite3)

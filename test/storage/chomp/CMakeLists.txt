##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/base-test)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/chomp)

##################################################################
# Link and Create Executable
##################################################################
add_executable(TestChomp test-chomp.cpp)
target_link_libraries(TestChomp BaseTest Chomp Nibble Star
        SQLiteCpp Sqlite3)

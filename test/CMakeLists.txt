##################################################################
# Binary Directory = bin Folder
##################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/test/bin)

##################################################################
# Build Base-Test Library
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/base-test)

##################################################################
# Add Subdirectory Containing User Tests
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/identification)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/storage)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math)

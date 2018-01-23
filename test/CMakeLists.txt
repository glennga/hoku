##################################################################
# Binary Directory = bin Folder
##################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/test/bin)

##################################################################
# Add Google Test Library
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/lib/googletest)

##################################################################
# Add Subdirectories Containing User Tests
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tests/identification)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tests/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tests/storage)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tests/math)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tests/experiment)
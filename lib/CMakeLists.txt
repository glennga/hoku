##################################################################
# Add Subdirectories Containing Third Party Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/third-party)

##################################################################
# Add Subdirectories Containing User Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/storage)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/trial)
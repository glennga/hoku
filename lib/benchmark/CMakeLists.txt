##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# User Configuration: Specify library name
##################################################################
set(LIBNAME Benchmark)

##################################################################
# User Configuration: Source and Include Files
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Add Library - Install List
##################################################################
add_library(${LIBNAME} STATIC ${SOURCES} ${INCLUDES})
install(TARGETS ${LIBNAME} DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
install(FILES ${INCLUDESPP} DESTINATION include)


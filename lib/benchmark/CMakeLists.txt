#
# Require CMake Version 3.7
#
cmake_minimum_required(VERSION 3.7)

#
# Include Directory for Header Files
#
include_directories(${CMAKE_SOURCE_DIR}/include)

#
# Build Library Benchmark
#
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/benchmark/benchmark.h)
add_library(Benchmark STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Benchmark DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

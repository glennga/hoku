cmake_minimum_required(VERSION 2.8.10)
include_directories(${CMAKE_SOURCE_DIR}/include)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/benchmark/benchmark.h)
add_library(Benchmark STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Benchmark DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

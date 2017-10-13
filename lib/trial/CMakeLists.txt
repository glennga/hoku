##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library Query
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/query.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/query.h)
add_library(Query STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Query DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Alignment
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/alignment.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/alignment.h)
add_library(Alignment STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Alignment DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)


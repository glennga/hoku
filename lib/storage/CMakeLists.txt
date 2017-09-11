##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library Nibble
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/nibble.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/nibble.h)
add_library(Nibble STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Nibble DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Chomp
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/chomp.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/chomp.h)
add_library(Chomp STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Chomp DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library KdNode
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/kd-node.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/kd-node.h)
add_library(KdNode STATIC ${SOURCES} ${INCLUDES})
install(TARGETS KdNode DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library QuadNode
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/quad-node.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/quad-node.h)
add_library(QuadNode STATIC ${SOURCES} ${INCLUDES})
install(TARGETS QuadNode DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

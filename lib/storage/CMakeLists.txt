#
# Require CMake Version 2.8.10
#
cmake_minimum_required(VERSION 2.8.10)

#
# Include Directory for Header Files
#
include_directories(${CMAKE_SOURCE_DIR}/include)

#
# Build Library Nibble
#
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/nibble.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/nibble.h)
add_library(Nibble STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Nibble DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

#
# Build Library Chomp
#
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/chomp.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/storage/chomp.h)
add_library(Chomp STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Chomp DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
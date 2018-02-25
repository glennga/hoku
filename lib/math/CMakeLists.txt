##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library RandomDraw
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/random-draw.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/math/random-draw.h)
add_library(RandomDraw STATIC ${SOURCES} ${INCLUDES})
install(TARGETS RandomDraw DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Star
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/star.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/math/star.h)
add_library(Star STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Star DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Rotation
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/rotation.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/math/rotation.h)
add_library(Rotation STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Rotation DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Trio
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/trio.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/math/trio.h)
add_library(Trio STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Trio DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Mercator
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mercator.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/math/mercator.h)
add_library(Mercator STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Mercator DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
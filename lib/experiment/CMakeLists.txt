##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library Lumberjack
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lumberjack.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/experiment/lumberjack.h)
add_library(Lumberjack STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Lumberjack DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Experiment
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/experiment.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/experiment/experiment.h)
add_library(Experiment STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Experiment DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library Trials
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/trial.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/trial.h)
add_library(Trial STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Trial DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

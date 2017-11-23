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

##################################################################
# Build Library Reduction
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reduction.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/reduction.h)
add_library(Reduction STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Reduction DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Semi-Crown
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/semi-crown.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/semi-crown.h)
add_library(SemiCrown STATIC ${SOURCES} ${INCLUDES})
install(TARGETS SemiCrown DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Crown
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/crown.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/trial/crown.h)
add_library(Crown STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Crown DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)


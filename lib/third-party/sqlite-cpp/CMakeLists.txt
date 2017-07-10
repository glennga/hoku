##################################################################
# CMakeList - Static Library Template
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# User Configuration: Specify library name
##################################################################
set(LIBNAME SQLiteCpp)

##################################################################
# User Configuration: Source and Include Files
##################################################################
# ----------------- Direct Source: This Library ------------------
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
FILE(GLOB INCLUDESPP ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
include_directories(..)

# --------- Additional Direct Source Below This line -------------
include_directories(sqlite3)

##################################################################
# Add Library - Install List
##################################################################
add_library(${LIBNAME} STATIC ${SOURCES} ${INCLUDES})
install(TARGETS ${LIBNAME} DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
install(FILES ${INCLUDESPP} DESTINATION include)


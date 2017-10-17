##################################################################
# Require CMake Version 3.7
##################################################################
cmake_minimum_required(VERSION 3.7)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Build Library Angle
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/angle.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/identification/angle.h)
add_library(Angle STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Angle DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library BaseTriangle
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/base-triangle.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/identification/base-triangle.h)
add_library(BaseTriangle STATIC ${SOURCES} ${INCLUDES})
install(TARGETS BaseTriangle DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library PlanarTriangle
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/planar-triangle.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/identification/planar-triangle.h)
add_library(PlanarTriangle STATIC ${SOURCES} ${INCLUDES})
install(TARGETS PlanarTriangle DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library SphericalTriangle
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/spherical-triangle.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/identification/spherical-triangle.h)
add_library(SphericalTriangle STATIC ${SOURCES} ${INCLUDES})
install(TARGETS SphericalTriangle DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)

##################################################################
# Build Library Pyramid
##################################################################
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pyramid.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/include/identification/pyramid.h)
add_library(Pyramid STATIC ${SOURCES} ${INCLUDES})
install(TARGETS Pyramid DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
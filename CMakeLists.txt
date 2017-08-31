##################################################################
# Project Name = HOKU
##################################################################
set(PROJECT_NAME HOKU)
cmake_minimum_required(VERSION 3.7)
project(${PROJECT_NAME})

##################################################################
# GPROF Profiling Builds
##################################################################
Option(BUILD_GPROF "Build for Profiling" OFF)
if (BUILD_GPROF)
    message("Building for GPrOF Profiler")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -Wall -pedantic \
	-sttd=c++0x -U__STRICT_ANSI__ -Wno-invalid-offsetof")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic \
	-std=c++0x -U__STRICT_ANSI__ -Wno-invalid-offsetof")
endif ()

##################################################################
# Static Executable Builds
##################################################################
Option(BUILD_STATIC "Build for Static Executables" ON)
if (BUILD_STATIC)
    message("Building Static Executables")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static \
        -static-libgcc -static-libstdc++")
    #   -static-libgcc -static-libstdc++ -Wl,--stack,2147483647")
endif ()

##################################################################
# Windows vs. Unix. Grr!!
##################################################################
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -m64")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -ldl")
endif ()

##################################################################
# Binary Directory = bin Folder
##################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

##################################################################
# Add Subdirectory Containing Third Party Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp/sqlite3)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)

##################################################################
# Add Subdirectory Containing User Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/base-test)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/star)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/rotation)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/trio)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/mercator)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/asterism)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tree/quad-node)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tree/kd-node)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/storage/chomp)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/angle)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/base-triangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/planar-triangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/spherical-triangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/astrometry-net)

##################################################################
# Add Subdirectory Containing User Tests
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/star)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/rotation)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/trio)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/mercator)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/asterism)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tree/quad-node)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/tree/kd-node)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/storage/nibble)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/storage/chomp)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/identification/angle)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/identification/planar-triangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/identification/spherical-triangle)

##################################################################
# Add Subdirectory Containing Source Files (the meat)
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/src/)
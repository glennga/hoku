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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -pg -g3 -Wall -pedantic \
	-std=c++11 -U__STRICT_ANSI__ -fno-omit-frame-pointer -gdwarf-2")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic \
	-std=c++11 -U__STRICT_ANSI__")
endif ()

##################################################################
# Static Executable Builds
##################################################################
Option(BUILD_STATIC "Build for Static Executables" OFF)
if (BUILD_STATIC)
    message("Building Static Executables")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static \
        -static-libgcc -static-libstdc++")
    #   -static-libgcc -static-libstdc++ -Wl,--stack,2147483647")
endif ()

##################################################################
# Threading in Unix and Windows
##################################################################
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -m64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -ldl")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl")
endif ()

##################################################################
# Add Subdirectories Containing Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)

##################################################################
# Add Subdirectory Containing User Tests (If Desired)
##################################################################
Option(BUILD_TEST "Build for Testing" OFF)
if (BUILD_TEST)
    message("Building Test Executables")
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif ()

##################################################################
# Add Subdirectory Containing Source Files (the meat)
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/src/)
##################################################################
# Project Name = HOKU
##################################################################
set(PROJECT_NAME HOKU)

##################################################################
# Type of Build: Choose Release or Debug
##################################################################
set(CMAKE_BUILD_TYPE Debug)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("Building Debug Version")
endif()
if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    message("Building Release Version")
endif()

# remove when CMake >= 2.8.4 is required
set(CMAKE_LEGACY_CYGWIN_WIN32 0) 
cmake_minimum_required(VERSION 3.7)
project(${PROJECT_NAME})

Option(BUILD_GPROF "Build for Profiling" OFF)
if (BUILD_GPROF)
    message("Building for GPrOF Profiler")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -Wall -pedantic \
	-sttd=c++0x -U__STRICT_ANSI__ -Wno-invalid-offsetof")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic \
	-std=c++0x -U__STRICT_ANSI__ -Wno-invalid-offsetof")
endif ()

Option(BUILD_STATIC "Build Static Executables" OFF)
if (BUILD_STATIC AND NOT MSVC)
    message("Building Static Executables")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static \
	-static-libgcc -static-libstdc++")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

##################################################################
# Add Subdirectory Containing Third Party Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp/sqlite3)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)

##################################################################
# Add Subdirectory Containing User Libraries
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/base_test)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/star)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/rotation)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/math/trio)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/nibble)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/identification/angle)

##################################################################
# Add Subdirectory Containing User Tests
##################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/star)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/rotation)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/math/trio)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/nibble)
add_subdirectory(${CMAKE_SOURCE_DIR}/test/benchmark)

set(PROJECT_NAME HOKU)
cmake_minimum_required(VERSION 2.8.10)

Option(BUILD_GPROF "Build for Profiling" OFF)
if (BUILD_GPROF)
    message("Building for GPrOF Profiler")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -ggdb -Wall -pedantic -std=c++14 -U__STRICT_ANSI__ \
    -fno-omit-frame-pointer -gdwarf-2 -Wextra ")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic -std=c++14 -U__STRICT_ANSI__")
endif ()

Option(BUILD_STATIC "Build for Static Executables" OFF)
if (BUILD_STATIC)
    message("Building Static Executables")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(${CMAKE_SOURCE_DIR}/lib)
set(HOKU_MATH_LIBS Mercator Rotation Trio Star RandomDraw)
set(HOKU_STORAGE_LIBS Chomp Nibble SQLiteCpp Sqlite3)
set(HOKU_BENCHMARK Benchmark)
set(HOKU_IDENTIFY_LIBS CompositePyramid Pyramid PlanarTriangle SphericalTriangle DotAngle Angle BaseTriangle
        Identification)
set(HOKU_LIBS ${HOKU_IDENTIFY_LIBS} ${HOKU_BENCHMARK} ${HOKU_STORAGE_LIBS} ${HOKU_MATH_LIBS})

Option(BUILD_TEST "Build for Testing" OFF)
if (BUILD_TEST)
    message("Building Test Executables")
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif ()

add_subdirectory(${CMAKE_SOURCE_DIR}/src/)
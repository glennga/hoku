##################################################################
# Binary Directory = bin Folder
##################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Link and Create Executable GenerateN
##################################################################
add_executable(GenerateN generate-n.cpp)
target_link_libraries(GenerateN Summer Pyramid PlanarTriangle
        SphericalTriangle Angle BaseTriangle Identification Benchmark
        Rotation QuadNode KdNode Chomp Nibble Asterism Mercator Trio
        Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable PerformE
##################################################################
add_executable(PerformE perform-e.cpp)
target_link_libraries(PerformE Experiment Lumberjack Summer Pyramid
        PlanarTriangle SphericalTriangle Angle BaseTriangle
        Identification Benchmark Rotation QuadNode KdNode Chomp
        Nibble Mercator Trio Star SQLiteCpp Sqlite3)
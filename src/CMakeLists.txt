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

target_link_libraries(GenerateN Pyramid AstrometryNet PlanarTriangle
        SphericalTriangle Angle BaseTriangle Benchmark Rotation QuadNode
        KdNode Chomp Nibble Asterism Mercator Trio Star SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable GenerateB
##################################################################
add_executable(GenerateB generate-b.cpp)
target_link_libraries(GenerateB Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable RunTrial
##################################################################
add_executable(RunTrial run-trial.cpp)
target_link_libraries(RunTrial Query Pyramid AstrometryNet PlanarTriangle
        SphericalTriangle Angle BaseTriangle Benchmark Rotation QuadNode
        KdNode Chomp Nibble Asterism Mercator Trio Star SQLiteCpp Sqlite3)
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
# Link and Create Executable TrialAngle
##################################################################
add_executable(TrialAngle trial-angle.cpp)
target_link_libraries(TrialAngle Angle Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TrialPlane
##################################################################
add_executable(TrialPlane trial-plane.cpp)
target_link_libraries(TrialPlane PlanarTriangle BaseTriangle Benchmark
        Rotation QuadNode Chomp Nibble Mercator Trio Star SQLiteCpp
        Sqlite3)

##################################################################
# Link and Create Executable TrialSphere
##################################################################
add_executable(TrialSphere trial-sphere.cpp)
target_link_libraries(TrialSphere SphericalTriangle BaseTriangle
        Benchmark Rotation QuadNode Chomp Nibble Mercator Trio Star
        SQLiteCpp Sqlite3)

##################################################################
# Link and Create Executable TrialAstro
##################################################################
add_executable(TrialAstro trial-astro.cpp)
target_link_libraries(TrialAstro AstrometryNet Benchmark Rotation
        KdNode Chomp Nibble Asterism Mercator Star SQLiteCpp Sqlite3)

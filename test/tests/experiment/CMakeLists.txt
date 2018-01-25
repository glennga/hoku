##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestLumberjack
##################################################################
add_executable(TestLumberjack test-lumberjack.cpp)
target_link_libraries(TestLumberjack gtest gmock Lumberjack Chomp
        Nibble Star SQLiteCpp Sqlite3 RandomDraw)

##################################################################
# Link and Create Executable TestExperiment
##################################################################
add_executable(TestExperiment test-experiment.cpp)
target_link_libraries(TestExperiment gtest gmock Experiment
        Lumberjack Pyramid PlanarTriangle SphericalTriangle Angle
        BaseTriangle Identification Benchmark Rotation QuadNode
        KdNode Chomp Nibble Mercator Trio Star SQLiteCpp Sqlite3
        RandomDraw)
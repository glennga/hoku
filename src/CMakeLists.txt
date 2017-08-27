##################################################################
# Include Directories for User Libraries
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/math/star)
include_directories(${CMAKE_SOURCE_DIR}/lib/third-party/sqlite-cpp)
include_directories(${CMAKE_SOURCE_DIR}/lib/storage/nibble)
include_directories(${CMAKE_SOURCE_DIR}/lib/math/rotation)
include_directories(${CMAKE_SOURCE_DIR}/lib/benchmark)

include_directories(${CMAKE_SOURCE_DIR}/lib/identification/angle)

##################################################################
# Link and Create Executables
##################################################################
add_executable(Generator generator.cpp)
target_link_libraries(Generator Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)

add_executable(TrialAngle trial-angle.cpp)
target_link_libraries(TrialAngle Angle Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)


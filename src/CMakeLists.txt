##################################################################
# Binary Directory = bin Folder
##################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Link and Create Executables
##################################################################
add_executable(Generator benchmark-generator.cpp)
target_link_libraries(Generator Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)

add_executable(TrialAngle trial-angle.cpp)
target_link_libraries(TrialAngle Angle Benchmark Rotation Nibble Star
        SQLiteCpp Sqlite3)


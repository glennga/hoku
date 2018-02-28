#
# Include Directory for Header Files (Source)
#
include_directories(${CMAKE_SOURCE_DIR}/include)

#
# Include Directory for Header Files
#
include_directories(${CMAKE_SOURCE_DIR}/test/include)

#
# Link and Create Executable TestLumberjack
#
add_executable(TestLumberjack test-lumberjack.cpp)
target_link_libraries(TestLumberjack gtest gmock ${HOKU_LIBS})

#
# Link and Create Executable TestExperiment
#
add_executable(TestExperiment test-experiment.cpp)
target_link_libraries(TestExperiment gtest gmock ${HOKU_LIBS})

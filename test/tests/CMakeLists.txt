#
# Include Directory for Header Files
#
include_directories(${CMAKE_SOURCE_DIR}/test/include)

#
# Include Directory for Header Files
#
include_directories(${CMAKE_SOURCE_DIR}/include)

#
# Link and Create the Test Runner Executable
#
add_executable(PerformT perform-t.cpp)
target_link_libraries(PerformT gtest gmock Experiment Lumberjack ${HOKU_LIBS})

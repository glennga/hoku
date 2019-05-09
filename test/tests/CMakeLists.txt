include_directories(${CMAKE_SOURCE_DIR}/test/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(PerformT perform-t.cpp)
target_link_libraries(PerformT gtest gmock Experiment Lumberjack ${HOKU_LIBS})

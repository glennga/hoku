set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(GenerateN generate-n.cpp)
target_link_libraries(GenerateN ${HOKU_LIBS})

add_executable(PerformE perform-e.cpp)
target_link_libraries(PerformE Experiment Lumberjack ${HOKU_LIBS})

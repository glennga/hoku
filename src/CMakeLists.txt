set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(GenerateN generate-n.cpp)
target_link_libraries(GenerateN ${HOKU_LIBS})

add_executable(PerformE perform-e.cpp)
target_link_libraries(PerformE Experiment Lumberjack ${HOKU_LIBS})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(ProcessI process-i.cpp)
target_link_libraries(ProcessI ${OpenCV_LIBS} ${HOKU_LIBS})

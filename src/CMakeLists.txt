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
target_link_libraries(GenerateN ${HOKU_LIBS})

##################################################################
# Link and Create Executable PerformE
##################################################################
add_executable(PerformE perform-e.cpp)
target_link_libraries(PerformE Experiment Lumberjack ${HOKU_LIBS})

##################################################################
# Link and Create Executable RunIdentify
##################################################################
add_executable(RunIdentify run-identify.cpp)
target_link_libraries(RunIdentify ${HOKU_LIBS})
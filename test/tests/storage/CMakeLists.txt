##################################################################
# Include Directory for Header Files (Source)
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/include)

##################################################################
# Include Directory for Header Files
##################################################################
include_directories(${CMAKE_SOURCE_DIR}/test/include)

##################################################################
# Link and Create Executable TestNibble
##################################################################
add_executable(TestNibble test-nibble.cpp)
target_link_libraries(TestNibble gtest ${HOKU_STORAGE_LIBS}
        ${HOKU_MATH_LIBS})

##################################################################
# Link and Create Executable TestChomp
##################################################################
add_executable(TestChomp test-chomp.cpp)
target_link_libraries(TestChomp gtest gmock ${HOKU_STORAGE_LIBS}
        ${HOKU_MATH_LIBS})
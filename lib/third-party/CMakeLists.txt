cmake_minimum_required(VERSION 2.8.10)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(Sqlite3 ${CMAKE_SOURCE_DIR}/lib/third-party/sqlite3/sqlite3.c
        ${CMAKE_SOURCE_DIR}/include/third-party/sqlite/sqlite3.h)
target_link_libraries(Sqlite3 dl)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-cpp/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_SOURCE_DIR}/sqlite-cpp/*.h)
add_library(SQLiteCpp STATIC ${SOURCES} ${INCLUDES})
install(TARGETS SQLiteCpp DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)
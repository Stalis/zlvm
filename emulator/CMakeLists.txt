cmake_minimum_required(VERSION 3.13)
project(zlvm C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_C_STANDARD 11)

file(GLOB EXPORT_H include/*.h)
file(GLOB HEADERS ${EXPORT_H} src/*.h)
file(GLOB SOURCES src/*.c)

add_library(vmobjlib OBJECT ${HEADERS} ${SOURCES})
set_property(TARGET vmobjlib PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(zlvm SHARED $<TARGET_OBJECTS:vmobjlib>)
add_library(zlvm-static STATIC $<TARGET_OBJECTS:vmobjlib>)
set_target_properties(zlvm zlvm-static
        PROPERTIES PUBLIC_HEADER "${EXPORT_H}")
target_link_libraries(zlvm zlvm-static zlasm)

add_executable(zlvm-bin main.c)
target_link_libraries(zlvm-bin zlvm)

set_target_properties(zlvm-bin zlvm-static
        PROPERTIES OUTPUT_NAME zlvm)

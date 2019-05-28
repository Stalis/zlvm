cmake_minimum_required(VERSION 3.13)
project(zlasm C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_C_STANDARD 11)

file(GLOB EXPORT_H include/*.h)
file(GLOB HEADERS ${EXPORT_H} src/*.h)
file(GLOB SOURCES src/*.c)

add_library(asmobjlib OBJECT ${HEADERS} ${SOURCES})
set_property(TARGET asmobjlib PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(zlasm SHARED $<TARGET_OBJECTS:asmobjlib>)
add_library(zlasm-static STATIC $<TARGET_OBJECTS:asmobjlib>)
target_link_libraries(zlasm zlasm-static zlvm)
set_target_properties(zlasm zlasm-static
        PROPERTIES PUBLIC_HEADER "${EXPORT_H}")

add_executable(zlasm-bin main.c)
target_link_libraries(zlasm-bin zlasm)

set_target_properties(zlasm-bin zlasm-static
        PROPERTIES OUTPUT_NAME zlasm)

file(GLOB ZLASM_PUBLIC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../include/asm/*.h")
file(GLOB ZLASM_PRIVATE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
file(GLOB ZLASM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")

add_library(asmobjlib OBJECT ${ZLASM_PUBLIC_HEADERS} ${ZLASM_PRIVATE_HEADERS} ${ZLASM_SOURCES})
set_target_properties(asmobjlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(
    asmobjlib
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include" "${CMAKE_CURRENT_LIST_DIR}/../include/asm"
)
zlvm_configure_target(asmobjlib)

add_library(zlasm SHARED $<TARGET_OBJECTS:asmobjlib>)
add_library(zlasm-static STATIC $<TARGET_OBJECTS:asmobjlib>)

set_target_properties(
    zlasm zlasm-static
    PROPERTIES PUBLIC_HEADER "${ZLASM_PUBLIC_HEADERS}"
)

target_include_directories(
    zlasm
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include" "${CMAKE_CURRENT_LIST_DIR}/../include/asm"
)
target_include_directories(
    zlasm-static
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include" "${CMAKE_CURRENT_LIST_DIR}/../include/asm"
)
target_link_libraries(zlasm PRIVATE zlasm-static zlvm)

add_executable(zlasm-bin main.c)
set_target_properties(zlasm-bin zlasm-static PROPERTIES OUTPUT_NAME zlasm)
target_include_directories(
    zlasm-bin
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include" "${CMAKE_CURRENT_LIST_DIR}/../include/asm"
)
target_link_libraries(zlasm-bin PRIVATE zlasm)
zlvm_configure_target(zlasm-bin)

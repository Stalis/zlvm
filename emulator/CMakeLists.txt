file(GLOB ZLVM_PUBLIC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../include/*.h")
file(GLOB ZLVM_PRIVATE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
file(GLOB ZLVM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")

add_library(vmobjlib OBJECT ${ZLVM_PUBLIC_HEADERS} ${ZLVM_PRIVATE_HEADERS} ${ZLVM_SOURCES})
set_target_properties(vmobjlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(vmobjlib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")
zlvm_configure_target(vmobjlib)

add_library(zlvm SHARED $<TARGET_OBJECTS:vmobjlib>)
add_library(zlvm-static STATIC $<TARGET_OBJECTS:vmobjlib>)

set_target_properties(
    zlvm zlvm-static
    PROPERTIES PUBLIC_HEADER "${ZLVM_PUBLIC_HEADERS}"
)

target_include_directories(zlvm PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")
target_include_directories(zlvm-static PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")
target_link_libraries(zlvm PRIVATE zlvm-static)

add_executable(zlvm-bin main.c)
set_target_properties(zlvm-bin zlvm-static PROPERTIES OUTPUT_NAME zlvm)
target_include_directories(zlvm-bin PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
target_link_libraries(zlvm-bin PRIVATE zlvm zlasm)
zlvm_configure_target(zlvm-bin)

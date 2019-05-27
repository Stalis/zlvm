cmake_minimum_required(VERSION 3.13)
project(zlvm_c C)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/zlvm/)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_C_STANDARD 11)

set(BIN_PATH bin)
set(LIB_PATH lib)
set(INCLUDE_PATH include)

add_subdirectory(emulator)
add_subdirectory(asm)

install(TARGETS zlvm zlvm-static zlvm-bin
        RUNTIME DESTINATION ${BIN_PATH}
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}/static
        PUBLIC_HEADER DESTINATION ${INCLUDE_PATH})

install(TARGETS zlasm zlasm-static zlasm-bin
        RUNTIME DESTINATION ${BIN_PATH}
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}/static
        PUBLIC_HEADER DESTINATION ${INCLUDE_PATH}/asm)

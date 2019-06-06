cmake_minimum_required(VERSION 3.13)
project(zlvm_c C)

set(BIN_PATH bin)
set(LIB_PATH lib)
set(INCLUDE_PATH include/zlvm)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
    if (NOT APPLE)
        set(CMAKE_INSTALL_PREFIX "/usr")
    endif ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wold-style-cast")
endif ()


IF (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install/zlvm/)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
ENDIF ()

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
include(PVS-Studio.cmake)

add_subdirectory(emulator)
add_subdirectory(asm)

pvs_studio_add_target(
        TARGET ${PROJECT_NAME}.analyze ALL
        COMPILE_COMMANDS
        OUTPUT FORMAT tasklist
        LOG target.tasks
)


install(TARGETS zlvm zlvm-static zlvm-bin
        RUNTIME DESTINATION ${BIN_PATH}
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        PUBLIC_HEADER DESTINATION ${INCLUDE_PATH})

install(TARGETS zlasm zlasm-static zlasm-bin
        RUNTIME DESTINATION ${BIN_PATH}
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        PUBLIC_HEADER DESTINATION ${INCLUDE_PATH}/asm)

cmake_minimum_required(VERSION 4.4.2)

project(zlvm VERSION 0.1.0 LANGUAGES C)

include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ZLVM_BIN_PATH bin)
set(ZLVM_LIB_PATH lib)
set(ZLVM_INCLUDE_PATH include/zlvm)

function(zlvm_configure_target target_name)
    target_compile_features(${target_name} PRIVATE c_std_11)
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
        target_compile_definitions(${target_name} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra)
    endif()
endfunction()

add_subdirectory(emulator)
add_subdirectory(asm)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(
    TARGETS zlvm zlvm-static zlvm-bin
    RUNTIME DESTINATION ${ZLVM_BIN_PATH}
    LIBRARY DESTINATION ${ZLVM_LIB_PATH}
    ARCHIVE DESTINATION ${ZLVM_LIB_PATH}
    PUBLIC_HEADER DESTINATION ${ZLVM_INCLUDE_PATH}
)

install(
    TARGETS zlasm zlasm-static zlasm-bin
    RUNTIME DESTINATION ${ZLVM_BIN_PATH}
    LIBRARY DESTINATION ${ZLVM_LIB_PATH}
    ARCHIVE DESTINATION ${ZLVM_LIB_PATH}
    PUBLIC_HEADER DESTINATION ${ZLVM_INCLUDE_PATH}/asm
)

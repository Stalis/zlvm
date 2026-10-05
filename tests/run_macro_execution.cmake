if(
    NOT DEFINED ZLVM_EXECUTABLE
    OR NOT DEFINED ZLASM_EXECUTABLE
    OR NOT DEFINED ZLVM_MACRO_TEST_PROGRAM
    OR NOT DEFINED ZLVM_MACRO_TEST_DIR
)
    message(FATAL_ERROR "The macro execution test paths are required")
endif()

file(MAKE_DIRECTORY "${ZLVM_MACRO_TEST_DIR}")
set(binary_path "${ZLVM_MACRO_TEST_DIR}/macro.bin")

execute_process(
    COMMAND "${ZLASM_EXECUTABLE}" "${ZLVM_MACRO_TEST_PROGRAM}" -o "${binary_path}"
    RESULT_VARIABLE assembler_result
    OUTPUT_VARIABLE assembler_output
    ERROR_VARIABLE assembler_error
)
if(NOT assembler_result EQUAL 0)
    message(
        FATAL_ERROR
        "Macro fixture assembly failed with status ${assembler_result}:\n"
        "${assembler_error}\n${assembler_output}"
    )
endif()

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" "${ZLVM_MACRO_TEST_PROGRAM}"
    RESULT_VARIABLE source_result
    OUTPUT_VARIABLE source_output
    ERROR_VARIABLE source_error
)
execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" --binary "${binary_path}"
    RESULT_VARIABLE binary_result
    OUTPUT_VARIABLE binary_output
    ERROR_VARIABLE binary_error
)

if(NOT source_result EQUAL 0 OR NOT binary_result EQUAL 0)
    message(
        FATAL_ERROR
        "Macro execution failed:\n"
        "source (${source_result}): ${source_error}\n${source_output}\n"
        "binary (${binary_result}): ${binary_error}\n${binary_output}"
    )
endif()
if(NOT source_output STREQUAL binary_output)
    message(
        FATAL_ERROR
        "Macro source and assembled binary output differ:\n"
        "source:\n${source_output}\n"
        "binary:\n${binary_output}"
    )
endif()
if(NOT source_output MATCHES "Halted")
    message(FATAL_ERROR "Macro fixture did not halt:\n${source_output}")
endif()

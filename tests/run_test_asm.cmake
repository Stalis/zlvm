if(
    NOT DEFINED ZLVM_EXECUTABLE
    OR NOT DEFINED ZLASM_EXECUTABLE
    OR NOT DEFINED ZLVM_TEST_PROGRAM
    OR NOT DEFINED ZLVM_TEST_DIR
)
    message(FATAL_ERROR "The ZLVM and ZLASM executables, test program, and test directory are required")
endif()

file(MAKE_DIRECTORY "${ZLVM_TEST_DIR}")
set(test_binary "${ZLVM_TEST_DIR}/test.bin")

execute_process(
    COMMAND "${ZLASM_EXECUTABLE}" "${ZLVM_TEST_PROGRAM}" -o "${test_binary}"
    RESULT_VARIABLE assembler_result
    OUTPUT_VARIABLE assembler_output
    ERROR_VARIABLE assembler_error
)
if(NOT assembler_result EQUAL 0)
    message(
        FATAL_ERROR
        "Unable to assemble test binary (${assembler_result}):\n${assembler_error}\n${assembler_output}"
    )
endif()

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" "${ZLVM_TEST_PROGRAM}"
    RESULT_VARIABLE source_result
    OUTPUT_VARIABLE source_output
    ERROR_VARIABLE source_error
)

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" --binary "${test_binary}"
    RESULT_VARIABLE binary_result
    OUTPUT_VARIABLE binary_output
    ERROR_VARIABLE binary_error
)

if(NOT source_result EQUAL 0)
    message(
        FATAL_ERROR
        "Source execution failed with status ${source_result}:\n${source_error}\n${source_output}"
    )
endif()
if(NOT binary_result EQUAL 0)
    message(
        FATAL_ERROR
        "Binary execution failed with status ${binary_result}:\n${binary_error}\n${binary_output}"
    )
endif()
if(NOT "${source_output}" STREQUAL "${binary_output}")
    message(FATAL_ERROR "Source and binary output differ:\n${source_output}\n---\n${binary_output}")
endif()
if(NOT "${source_error}" STREQUAL "${binary_error}")
    message(FATAL_ERROR "Source and binary errors differ:\n${source_error}\n---\n${binary_error}")
endif()

foreach(expected_text IN ITEMS "Hello, World!" "Bye!" "Halted")
    string(FIND "${source_output}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Missing expected output '${expected_text}':\n${source_output}")
    endif()
endforeach()

function(expect_cli_failure name expected_error)
    execute_process(
        COMMAND "${ZLVM_EXECUTABLE}" ${ARGN}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(result EQUAL 0)
        message(FATAL_ERROR "${name} unexpectedly succeeded:\n${error}\n${output}")
    endif()
    string(FIND "${error}" "${expected_error}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${name} did not report '${expected_error}':\n${error}\n${output}")
    endif()
    string(FIND "${output}" "Result code:" result_index)
    if(NOT result_index EQUAL -1)
        message(FATAL_ERROR "${name} executed the VM:\n${error}\n${output}")
    endif()
endfunction()

expect_cli_failure("no arguments" "Usage:")
expect_cli_failure("missing binary path" "Usage:" --binary)
expect_cli_failure("malformed binary option" "Usage:" "--binary=${test_binary}")
expect_cli_failure("extra source argument" "Usage:" "${ZLVM_TEST_PROGRAM}" extra)
expect_cli_failure("extra binary argument" "Usage:" --binary "${test_binary}" extra)

set(missing_binary "${ZLVM_TEST_DIR}/missing.bin")
file(REMOVE "${missing_binary}")
expect_cli_failure("missing binary" "${missing_binary}" --binary "${missing_binary}")
expect_cli_failure("unavailable binary" "${ZLVM_TEST_DIR}" --binary "${ZLVM_TEST_DIR}")

set(empty_binary "${ZLVM_TEST_DIR}/empty.bin")
file(WRITE "${empty_binary}" "")
expect_cli_failure("empty binary" "Binary image is empty" --binary "${empty_binary}")

set(full_source "${ZLVM_TEST_DIR}/full.asm")
set(full_binary "${ZLVM_TEST_DIR}/full.bin")
file(WRITE "${full_source}" "int 0xFF\n.space 4088\n")
execute_process(
    COMMAND "${ZLASM_EXECUTABLE}" "${full_source}" -o "${full_binary}"
    RESULT_VARIABLE full_assembler_result
    OUTPUT_VARIABLE full_assembler_output
    ERROR_VARIABLE full_assembler_error
)
if(NOT full_assembler_result EQUAL 0)
    message(
        FATAL_ERROR
        "Unable to assemble 4096-byte fixture (${full_assembler_result}):\n"
        "${full_assembler_error}\n${full_assembler_output}"
    )
endif()
file(SIZE "${full_binary}" full_binary_size)
if(NOT full_binary_size EQUAL 4096)
    message(FATAL_ERROR "Expected a 4096-byte fixture, got ${full_binary_size} bytes")
endif()
execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" --binary "${full_binary}"
    RESULT_VARIABLE full_result
    OUTPUT_VARIABLE full_output
    ERROR_VARIABLE full_error
)
if(NOT full_result EQUAL 0 OR NOT full_output MATCHES "Halted")
    message(FATAL_ERROR "4096-byte fixture failed with status ${full_result}:\n${full_error}\n${full_output}")
endif()

set(oversized_binary "${ZLVM_TEST_DIR}/oversized.bin")
string(REPEAT "x" 4097 oversized_contents)
file(WRITE "${oversized_binary}" "${oversized_contents}")
expect_cli_failure("oversized binary" "Binary image exceeds ROM size" --binary "${oversized_binary}")

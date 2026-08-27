if(
    NOT DEFINED ZLVM_EXECUTABLE
    OR NOT DEFINED ZLASM_EXECUTABLE
    OR NOT DEFINED ZLVM_TEST_PROGRAM
    OR NOT DEFINED ZLVM_TEST_BINARY
    OR NOT DEFINED ZLVM_TEST_DIR
)
    message(
        FATAL_ERROR
        "The ZLVM and ZLASM executables, test program, test binary, and test directory are required"
    )
endif()

file(MAKE_DIRECTORY "${ZLVM_TEST_DIR}")
set(source_stdout "${ZLVM_TEST_DIR}/source.stdout")
set(source_stderr "${ZLVM_TEST_DIR}/source.stderr")
set(binary_stdout "${ZLVM_TEST_DIR}/binary.stdout")
set(binary_stderr "${ZLVM_TEST_DIR}/binary.stderr")

function(compare_output name source_path binary_path)
    file(READ "${source_path}" source_hex HEX)
    file(READ "${binary_path}" binary_hex HEX)
    if(source_hex STREQUAL binary_hex)
        return()
    endif()

    string(LENGTH "${source_hex}" source_hex_length)
    string(LENGTH "${binary_hex}" binary_hex_length)
    math(EXPR source_size "${source_hex_length} / 2")
    math(EXPR binary_size "${binary_hex_length} / 2")
    if(source_size LESS binary_size)
        set(common_size "${source_size}")
    else()
        set(common_size "${binary_size}")
    endif()

    set(offset 0)
    while(offset LESS common_size)
        math(EXPR hex_offset "${offset} * 2")
        string(SUBSTRING "${source_hex}" ${hex_offset} 2 source_byte)
        string(SUBSTRING "${binary_hex}" ${hex_offset} 2 binary_byte)
        if(NOT source_byte STREQUAL binary_byte)
            break()
        endif()
        math(EXPR offset "${offset} + 1")
    endwhile()

    math(EXPR context_start "${offset} - 8")
    if(context_start LESS 0)
        set(context_start 0)
    endif()
    math(EXPR context_hex_start "${context_start} * 2")
    math(EXPR source_context_length "${source_hex_length} - ${context_hex_start}")
    math(EXPR binary_context_length "${binary_hex_length} - ${context_hex_start}")
    if(source_context_length GREATER 32)
        set(source_context_length 32)
    endif()
    if(binary_context_length GREATER 32)
        set(binary_context_length 32)
    endif()
    string(SUBSTRING "${source_hex}" ${context_hex_start} ${source_context_length} source_context)
    string(SUBSTRING "${binary_hex}" ${context_hex_start} ${binary_context_length} binary_context)

    message(
        FATAL_ERROR
        "${name} differs at byte ${offset} "
        "(source size ${source_size}, binary size ${binary_size}); "
        "bytes from offset ${context_start}: source=${source_context}, binary=${binary_context}"
    )
endfunction()

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" "${ZLVM_TEST_PROGRAM}"
    RESULT_VARIABLE source_result
    OUTPUT_FILE "${source_stdout}"
    ERROR_FILE "${source_stderr}"
)

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" --binary "${ZLVM_TEST_BINARY}"
    RESULT_VARIABLE binary_result
    OUTPUT_FILE "${binary_stdout}"
    ERROR_FILE "${binary_stderr}"
)

file(READ "${source_stdout}" source_output)
file(READ "${source_stderr}" source_error)
file(READ "${binary_stdout}" binary_output)
file(READ "${binary_stderr}" binary_error)

if(NOT source_result STREQUAL binary_result)
    message(FATAL_ERROR "Process status differs: source=${source_result}, binary=${binary_result}")
endif()
if(NOT source_result EQUAL 0)
    message(
        FATAL_ERROR
        "Source and binary execution failed with status ${source_result}:\n"
        "source stderr:\n${source_error}\nsource stdout:\n${source_output}\n"
        "binary stderr:\n${binary_error}\nbinary stdout:\n${binary_output}"
    )
endif()
compare_output("stdout" "${source_stdout}" "${binary_stdout}")
compare_output("stderr" "${source_stderr}" "${binary_stderr}")

set(expected_program_output "Hello, World!\n\nBye!\n\n")
string(FIND "${source_output}" "${expected_program_output}" program_output_index)
if(program_output_index EQUAL -1)
    message(FATAL_ERROR "Missing exact program output 'Hello, World!\\n\\nBye!\\n\\n':\n${source_output}")
endif()

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
expect_cli_failure("extra source argument" "Usage:" "${ZLVM_TEST_PROGRAM}" extra)
expect_cli_failure("extra binary argument" "Usage:" --binary "${ZLVM_TEST_BINARY}" extra)

set(dash_source "${ZLVM_TEST_DIR}/-test.asm")
file(COPY_FILE "${ZLVM_TEST_PROGRAM}" "${dash_source}")
execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" -test.asm
    WORKING_DIRECTORY "${ZLVM_TEST_DIR}"
    RESULT_VARIABLE dash_source_result
    OUTPUT_VARIABLE dash_source_output
    ERROR_VARIABLE dash_source_error
)
if(NOT dash_source_result EQUAL 0 OR NOT "${dash_source_output}" STREQUAL "${source_output}")
    message(
        FATAL_ERROR
        "Leading-dash source failed with status ${dash_source_result}:\n"
        "${dash_source_error}\n${dash_source_output}"
    )
endif()

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

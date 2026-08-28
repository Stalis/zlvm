file(MAKE_DIRECTORY "${ZLASM_TEST_DIR}")

function(check_failure NAME SOURCE EXPECTED)
    set(SOURCE_PATH "${ZLASM_TEST_DIR}/${NAME}.asm")
    set(OUTPUT_PATH "${ZLASM_TEST_DIR}/${NAME}.bin")
    file(WRITE "${SOURCE_PATH}" "${SOURCE}")
    file(REMOVE "${OUTPUT_PATH}")

    foreach(EXECUTABLE IN ITEMS "${ZLASM_EXECUTABLE}" "${ZLVM_EXECUTABLE}")
        execute_process(
            COMMAND "${EXECUTABLE}" "${SOURCE_PATH}"
            RESULT_VARIABLE RESULT
            OUTPUT_VARIABLE OUTPUT
            ERROR_VARIABLE ERROR
        )
        if(RESULT EQUAL 0)
            message(FATAL_ERROR "${EXECUTABLE} accepted invalid ${NAME} input")
        endif()
        string(FIND "${ERROR}" "${SOURCE_PATH}${EXPECTED}" MATCH_INDEX)
        if(MATCH_INDEX EQUAL -1)
            message(FATAL_ERROR "Unexpected ${NAME} diagnostic from ${EXECUTABLE}:\n${ERROR}")
        endif()
        if(OUTPUT MATCHES "Result code:")
            message(FATAL_ERROR "${EXECUTABLE} executed invalid ${NAME} input:\n${OUTPUT}")
        endif()
    endforeach()

    if(EXISTS "${OUTPUT_PATH}")
        message(FATAL_ERROR "zlasm created output for invalid ${NAME} input: ${OUTPUT_PATH}")
    endif()
endfunction()

check_failure(
    lexer
    " \n    \"oops"
    ":2:5: error ZLASM0003: Unterminated string literal [bytes 6..11)"
)
check_failure(
    parser
    "movi $t0, 1 extra\n"
    ":1:13: error ZLASM0004: Unexpected trailing token [bytes 12..17)"
)
check_failure(
    directive
    ".global\n"
    ":1:1: error ZLASM0006: Invalid directive argument count [bytes 0..7)"
)
check_failure(
    numeric
    "movi $t0, 18446744073709551616\n"
    ":1:11: error ZLASM0008: Numeric literal overflows 64 bits [bytes 10..30)"
)

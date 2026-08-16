if(NOT DEFINED ZLVM_EXECUTABLE OR NOT DEFINED ZLVM_TEST_PROGRAM)
    message(FATAL_ERROR "The ZLVM executable and test program are required")
endif()

execute_process(
    COMMAND "${ZLVM_EXECUTABLE}" "${ZLVM_TEST_PROGRAM}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "test.asm failed with status ${result}:\n${error}\n${output}")
endif()

foreach(expected_text IN ITEMS "Hello, World!" "Bye!" "Halted")
    string(FIND "${output}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Missing expected output '${expected_text}':\n${output}")
    endif()
endforeach()

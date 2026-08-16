add_test(
    NAME zlvm.test-asm
    COMMAND
        ${CMAKE_COMMAND}
        -DZLVM_EXECUTABLE=$<TARGET_FILE:zlvm-bin>
        -DZLVM_TEST_PROGRAM=${CMAKE_SOURCE_DIR}/test.asm
        -P ${CMAKE_CURRENT_LIST_DIR}/run_test_asm.cmake
)

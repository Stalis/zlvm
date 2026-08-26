add_test(
    NAME zlvm.test-asm
    COMMAND
        ${CMAKE_COMMAND}
        -DZLVM_EXECUTABLE=$<TARGET_FILE:zlvm-bin>
        -DZLVM_TEST_PROGRAM=${CMAKE_SOURCE_DIR}/test.asm
        -P ${CMAKE_CURRENT_LIST_DIR}/run_test_asm.cmake
)

add_executable(zlvm-instruction-tests instruction_tests.c)
target_include_directories(zlvm-instruction-tests PRIVATE ${CMAKE_SOURCE_DIR}/emulator/src)
target_link_libraries(zlvm-instruction-tests PRIVATE zlasm-static zlvm-static)
zlvm_configure_target(zlvm-instruction-tests)
add_test(NAME zlvm.instructions COMMAND zlvm-instruction-tests)

add_test(
    NAME zlasm.invalid-operands
    COMMAND
        $<TARGET_FILE:zlasm-bin> ${CMAKE_CURRENT_LIST_DIR}/invalid_operands.asm -o
        ${CMAKE_CURRENT_BINARY_DIR}/invalid_operands.bin
)
set_tests_properties(zlasm.invalid-operands PROPERTIES WILL_FAIL TRUE)

set(ZLVM_TEST_BINARY ${CMAKE_CURRENT_BINARY_DIR}/test.bin)

add_test(
    NAME zlasm.test-asm-fixture
    COMMAND
        $<TARGET_FILE:zlasm-bin> ${CMAKE_SOURCE_DIR}/test.asm -o ${ZLVM_TEST_BINARY}
)
set_tests_properties(zlasm.test-asm-fixture PROPERTIES FIXTURES_SETUP zlvm-test-asm-binary)

add_test(
    NAME zlvm.test-asm
    COMMAND
        ${CMAKE_COMMAND}
        -DZLVM_EXECUTABLE=$<TARGET_FILE:zlvm-bin>
        -DZLASM_EXECUTABLE=$<TARGET_FILE:zlasm-bin>
        -DZLVM_TEST_PROGRAM=${CMAKE_SOURCE_DIR}/test.asm
        -DZLVM_TEST_BINARY=${ZLVM_TEST_BINARY}
        -DZLVM_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-fixtures
        -P ${CMAKE_CURRENT_LIST_DIR}/run_test_asm.cmake
)
set_tests_properties(zlvm.test-asm PROPERTIES FIXTURES_REQUIRED zlvm-test-asm-binary)

add_executable(zlvm-execution-equivalence-tests execution_equivalence_tests.c)
target_include_directories(
    zlvm-execution-equivalence-tests
    PRIVATE ${CMAKE_SOURCE_DIR}/emulator/src
)
target_link_libraries(zlvm-execution-equivalence-tests PRIVATE zlasm-static zlvm-static)
zlvm_configure_target(zlvm-execution-equivalence-tests)
add_test(
    NAME zlvm.execution-equivalence
    COMMAND
        zlvm-execution-equivalence-tests ${CMAKE_SOURCE_DIR}/test.asm ${ZLVM_TEST_BINARY}
)
set_tests_properties(zlvm.execution-equivalence PROPERTIES FIXTURES_REQUIRED zlvm-test-asm-binary)

add_executable(zlvm-instruction-tests instruction_tests.c)
target_include_directories(zlvm-instruction-tests PRIVATE ${CMAKE_SOURCE_DIR}/emulator/src)
target_link_libraries(zlvm-instruction-tests PRIVATE zlasm-static zlvm-static)
zlvm_configure_target(zlvm-instruction-tests)
add_test(NAME zlvm.instructions COMMAND zlvm-instruction-tests)

add_executable(zlasm-diagnostic-tests assembler_diagnostic_tests.c)
target_link_libraries(zlasm-diagnostic-tests PRIVATE zlasm-static zlvm-static)
zlvm_configure_target(zlasm-diagnostic-tests)
add_test(NAME zlasm.diagnostics COMMAND zlasm-diagnostic-tests)

add_test(
    NAME zlasm.cli-diagnostics
    COMMAND
        ${CMAKE_COMMAND}
        -DZLVM_EXECUTABLE=$<TARGET_FILE:zlvm-bin>
        -DZLASM_EXECUTABLE=$<TARGET_FILE:zlasm-bin>
        -DZLASM_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/diagnostic-fixtures
        -P ${CMAKE_CURRENT_LIST_DIR}/run_cli_diagnostics.cmake
)

add_test(
    NAME zlasm.invalid-operands
    COMMAND
        $<TARGET_FILE:zlasm-bin> ${CMAKE_CURRENT_LIST_DIR}/invalid_operands.asm -o
        ${CMAKE_CURRENT_BINARY_DIR}/invalid_operands.bin
)
set_tests_properties(zlasm.invalid-operands PROPERTIES WILL_FAIL TRUE)

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

#include "asm/zlasm.h"

static void expect_failure(const char *source, ZlasmDiagnosticCode code, size_t line, size_t column,
                           size_t byte_offset, size_t byte_length) {
    ZlasmResult result = zlasm_assemble(source, "diagnostic.asm");
    assert(result.binary == NULL);
    assert(result.binary_size == 0);
    assert(result.diagnostic.code == code);
    assert(result.diagnostic.message != NULL);
    assert(strcmp(result.diagnostic.source_filename, "diagnostic.asm") == 0);
    assert(result.diagnostic.has_source_location);
    assert(result.diagnostic.line == line);
    assert(result.diagnostic.column == column);
    assert(result.diagnostic.byte_offset == byte_offset);
    assert(result.diagnostic.byte_length == byte_length);
    zlasm_result_free(&result);
}

int main(void) {
    expect_failure(" \n    \"oops", ZLASM_DIAGNOSTIC_UNTERMINATED_LITERAL, 2, 5, 6, 5);
    expect_failure("movi $t0, 1 extra\n", ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN, 1, 13, 12, 5);
    expect_failure(".global\n", ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, 1, 1, 0, 7);
    expect_failure(".global 4\n", ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, 1, 9, 8, 1);
    expect_failure("movi $t0, 0xGG\n", ZLASM_DIAGNOSTIC_MALFORMED_NUMBER, 1, 11, 10, 4);
    expect_failure("movi $t0, 18446744073709551616\n", ZLASM_DIAGNOSTIC_NUMBER_OVERFLOW, 1, 11, 10,
                   20);
    expect_failure(".byte 256\n", ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE, 1, 7, 6, 3);
    expect_failure(".ascii 256\n", ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE, 1, 8, 7, 3);
    expect_failure(".asciiz 256\n", ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE, 1, 9, 8, 3);
    expect_failure("    unknown\n", ZLASM_DIAGNOSTIC_UNKNOWN_OPCODE, 1, 5, 4, 7);
    expect_failure("movi $r, 1\n", ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER, 1, 6, 5, 2);

    ZlasmResult result = zlasm_assemble(NULL, "invalid.asm");
    assert(result.diagnostic.code == ZLASM_DIAGNOSTIC_INVALID_ARGUMENT);
    assert(!result.diagnostic.has_source_location);
    zlasm_result_free(&result);

    size_t compatibility_size = 123;
    assert(assemblySource(".space \"bad\"\n", &compatibility_size) == NULL);
    assert(compatibility_size == 0);

    result = zlasm_assemble(".dword 18446744073709551615\n", "valid.asm");
    assert(result.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    assert(result.binary_size == 8);
    for (size_t index = 0; index < result.binary_size; index++) {
        assert(result.binary[index] == 0xFF);
    }
    zlasm_result_free(&result);

    result = zlasm_assemble(".extern factorial, 0xFF\nint 0xFF\n", "valid.asm");
    assert(result.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    assert(result.binary != NULL);
    assert(result.binary_size == 8);
    zlasm_result_free(&result);
    return 0;
}

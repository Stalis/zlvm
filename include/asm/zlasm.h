#ifndef ZLVM_ASM_ZLASM_H
#define ZLVM_ASM_ZLASM_H

#include "../Types.h"

typedef enum ZlasmDiagnosticCode {
    ZLASM_DIAGNOSTIC_NONE = 0,
    ZLASM_DIAGNOSTIC_INVALID_ARGUMENT = 1,
    ZLASM_DIAGNOSTIC_OUT_OF_MEMORY = 2,
    ZLASM_DIAGNOSTIC_UNTERMINATED_LITERAL = 3,
    ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN = 4,
    ZLASM_DIAGNOSTIC_UNKNOWN_DIRECTIVE = 5,
    ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE = 6,
    ZLASM_DIAGNOSTIC_MALFORMED_NUMBER = 7,
    ZLASM_DIAGNOSTIC_NUMBER_OVERFLOW = 8,
    ZLASM_DIAGNOSTIC_UNKNOWN_OPCODE = 9,
    ZLASM_DIAGNOSTIC_INVALID_OPERANDS = 10,
    ZLASM_DIAGNOSTIC_UNKNOWN_CONDITION = 11,
    ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER = 12,
    ZLASM_DIAGNOSTIC_UNKNOWN_LABEL = 13,
    ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE = 14,
    ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE = 15,
    ZLASM_DIAGNOSTIC_INTERNAL_ERROR = 16,
    ZLASM_DIAGNOSTIC_MALFORMED_LITERAL = 17,
} ZlasmDiagnosticCode;

typedef struct ZlasmDiagnostic {
    ZlasmDiagnosticCode code;
    const char *message;
    const char *source_filename;
    bool has_source_location;
    size_t line;
    size_t column;
    size_t byte_offset;
    size_t byte_length;
} ZlasmDiagnostic;

typedef struct ZlasmResult {
    uint8_t *binary;
    size_t binary_size;
    ZlasmDiagnostic diagnostic;
} ZlasmResult;

/**
 * Assemble NUL-terminated source without terminating the calling process.
 *
 * source_filename is borrowed and may be NULL, in which case "<memory>" is reported. The result
 * owns binary on success; release it with zlasm_result_free(). Diagnostic messages are static and
 * must not be freed. When has_source_location is true, line and column are 1-based and byte_offset
 * and byte_length describe a zero-based half-open source range. A failure has a NULL binary and a
 * nonzero diagnostic code.
 */
ZlasmResult zlasm_assemble(const char *source, const char *source_filename);

/** Release the binary owned by result and reset it to an empty result. */
void zlasm_result_free(ZlasmResult *result);

/**
 * Assemble a NUL-terminated source buffer.
 *
 * The caller retains ownership of source and owns the returned buffer. The returned buffer must be
 * released with free(). On success, binary_size receives its size in bytes. On failure, this
 * compatibility wrapper returns NULL and sets binary_size to zero.
 */
uint8_t *assemblySource(char *source, size_t *binary_size);

#endif // ZLVM_ASM_ZLASM_H

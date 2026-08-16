#ifndef ZLVM_ASM_ZLASM_H
#define ZLVM_ASM_ZLASM_H

#include "Types.h"

/**
 * Assemble a NUL-terminated source buffer.
 *
 * The caller retains ownership of source and owns the returned buffer. The returned buffer must be
 * released with free(). On success, binary_size receives its size in bytes.
 */
uint8_t *assemblySource(char *source, size_t *binary_size);

#endif // ZLVM_ASM_ZLASM_H

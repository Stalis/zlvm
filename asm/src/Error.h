#ifndef ZLVM_ASM_ERROR_H
#define ZLVM_ASM_ERROR_H

#include "Types.h"

#include <stdio.h>
#include <stdlib.h>

#define ZLASM_CRASH(message) asm_crash((message), __FILE__, __LINE__)
#define ZLASM_TOKEN_CRASH(message, token)                                                          \
    asm_token_crash((message), (token)->pos, (token)->line, (token)->col, (token)->value,          \
                    __FILE__, __LINE__)

static inline _Noreturn void asm_token_crash(const char *message, size_t position, size_t line,
                                             size_t column, const char *value,
                                             const char *source_file, size_t source_line) {
    fprintf(stderr, "Error at %s:%zu\n\t(%4zu:%3zu:%2zu): %s (%s)\n", source_file, source_line,
            position, line, column, message, value);
    exit(EXIT_FAILURE);
}

static inline _Noreturn void asm_crash(const char *message, const char *source_file,
                                       size_t source_line) {
    fprintf(stderr, "Crash at %s:%zu: %s\n", source_file, source_line, message);
    exit(EXIT_FAILURE);
}

#endif // ZLVM_ASM_ERROR_H

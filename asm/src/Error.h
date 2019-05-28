//
// Created by Stanislav on 2019-05-06.
//

#ifndef ZLASM_C_ERROR_H
#define ZLASM_C_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include "../../emulator/include/Types.h"

#define ZLASM__CRASH(msg) _crash(msg, __FILE__, __LINE__)
#define ZLASM__TOKEN_CRASH(msg, token) \
    _token_crash(msg, (token->pos), (token->line), (token->col), __FILE__, __LINE__)

static void _token_crash(const char* msg, size_t pos, size_t line, size_t col, const char* __file, size_t __line) {
    fprintf(stderr, "Error at %s:%lu\n\t (%4lu:%3lu:%2lu): %s",
            __file, __line, pos, line, col, msg);
    exit(-2);
}

static void _crash(const char* msg, const char* file, size_t line) {
    fprintf(stderr, "Crash at %s:%lu: %s", file, line, msg);
    exit(-1);
}

static inline void _errorCode(const char* msg, word code) {
    fprintf(stderr, "Error: %s", msg);
    exit(code);
}

static inline void _error(const char* msg) {
    _errorCode(msg, 0);
}

#endif //ZLASM_C_ERROR_H

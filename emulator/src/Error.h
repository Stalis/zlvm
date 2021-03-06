//
// Created by Stanislav on 2019-05-06.
//

#ifndef ZLVM_C_ERROR_H
#define ZLVM_C_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include "Types.h"

#ifndef __dead2
#define __dead2
#endif

#define ZLVM__CRASH(msg) _crash(msg, __FILE__, __LINE__)

static void _crash(const char* msg, const char* file, size_t line) __dead2 {
    fprintf(stderr, "Crash at %s:%lu: %s", file, line, msg);
    exit(-1);
}

static inline void _errorCode(const char* msg, word code) __dead2 {
    fprintf(stderr, "Error: %s", msg);
    exit(code);
}

static inline void _error(const char* msg) {
    _errorCode(msg, 0);
}

#endif //ZLVM_C_ERROR_H

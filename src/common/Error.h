//
// Created by Stanislav on 2019-05-06.
//

#ifndef ZLVM_C_ERROR_H
#define ZLVM_C_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include "Types.h"

static inline void _errorCode(const char* msg, word code) {
    fprintf(stderr, "Error: %s", msg);
    exit(code);
}

static inline void _error(const char* msg) {
    _errorCode(msg, 0);
}

#endif //ZLVM_C_ERROR_H

//
// Created by Stanislav on 2019-06-03.
//

#ifndef ZLVM_C_MEMORY_H
#define ZLVM_C_MEMORY_H

#include "Error.h"

static inline void* malloc_s(size_t __size) {
    void* buf = malloc(__size);
    if (buf == NULL) ZLVM__CRASH("Allocation error");
    return buf;
}

static inline void* realloc_s(void* __ptr, size_t __size) {
    void* buf = realloc(__ptr, __size);
    if (buf == NULL) ZLVM__CRASH("Reallocation error");
    return buf;
}

static inline void free_s(void* __ptr) {
    if (__ptr != NULL) free(__ptr);
}

#endif //ZLVM_C_MEMORY_H

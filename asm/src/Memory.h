//
// Created by Stanislav on 2019-06-03.
//

#ifndef ZLVM_C_MEMORY_H
#define ZLVM_C_MEMORY_H

#include "Error.h"

static inline void* malloc_s(size_t __size) {
    void* buf = malloc(__size);
    if (buf == NULL) ZLASM__CRASH("Allocation error");
    return buf;
}

static inline void* calloc_s(size_t __count, size_t __size) {
    void* buf = calloc(__count, __size);
    if (buf == NULL) ZLASM__CRASH("Allocation error");
    return buf;
}

static inline void* realloc_s(void* __ptr, size_t __size) {
    assert(__ptr != NULL);
    void* buf = realloc(__ptr, __size);
    if (buf == NULL) ZLASM__CRASH("Reallocation error");
    return buf;
}

static inline char* strdup_s(const char* __s) {
    assert(__s != NULL);
    char* buf = strdup(__s);
    if (buf == NULL) ZLASM__CRASH("String duplication error");
    return buf;
}

static inline void free_s(void* __ptr) {
    assert(__ptr != NULL);
    free(__ptr);
}

#define malloc(__size) malloc_s(__size)
#define calloc(__count, __size) calloc_s(__count, __size)
#define realloc(__ptr, __size) realloc_s(__ptr, __size)
#define strdup(__s) strdup_s(__s)
#define free(__ptr) free_s(__ptr); __ptr = NULL

#endif //ZLVM_C_MEMORY_H

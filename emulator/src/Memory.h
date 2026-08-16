#ifndef ZLVM_EMULATOR_MEMORY_H
#define ZLVM_EMULATOR_MEMORY_H

#include "Error.h"

#include <stdlib.h>

static inline void* vm_malloc(size_t size) {
    void* result = malloc(size == 0 ? 1 : size);
    if (result == NULL) {
        ZLVM_CRASH("Allocation error");
    }
    return result;
}

static inline void* vm_calloc(size_t count, size_t size) {
    void* result = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (result == NULL) {
        ZLVM_CRASH("Allocation error");
    }
    return result;
}

static inline void* vm_realloc(void* pointer, size_t size) {
    void* result = realloc(pointer, size == 0 ? 1 : size);
    if (result == NULL) {
        ZLVM_CRASH("Reallocation error");
    }
    return result;
}

#endif // ZLVM_EMULATOR_MEMORY_H

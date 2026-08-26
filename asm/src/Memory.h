#ifndef ZLVM_ASM_MEMORY_H
#define ZLVM_ASM_MEMORY_H

#include "Error.h"

#include <stdlib.h>
#include <string.h>

static inline void *asm_malloc(size_t size) {
    void *result = malloc(size == 0 ? 1 : size);
    if (result == NULL) {
        ZLASM_CRASH("Allocation error");
    }
    return result;
}

static inline void *asm_calloc(size_t count, size_t size) {
    void *result = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (result == NULL) {
        ZLASM_CRASH("Allocation error");
    }
    return result;
}

static inline void *asm_realloc(void *pointer, size_t size) {
    void *result = realloc(pointer, size == 0 ? 1 : size);
    if (result == NULL) {
        ZLASM_CRASH("Reallocation error");
    }
    return result;
}

static inline char *asm_strdup(const char *string) {
    if (string == NULL) {
        ZLASM_CRASH("Cannot duplicate a null string");
    }

    size_t size = strlen(string) + 1;
    char *result = asm_malloc(size);
    memcpy(result, string, size);
    return result;
}

static inline void asm_free(void *pointer) {
    free(pointer);
}

#endif // ZLVM_ASM_MEMORY_H

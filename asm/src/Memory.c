#include "Memory.h"

#include <stdlib.h>
#include <string.h>

struct AsmAllocation {
    void *pointer;
    struct AsmAllocation *next;
};

static void track(void *pointer) {
    AsmErrorContext *context = asm_error_context();
    if (context == NULL) {
        return;
    }

    AsmAllocation *allocation = malloc(sizeof *allocation);
    if (allocation == NULL) {
        free(pointer);
        asm_fail(ZLASM_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
    }
    allocation->pointer = pointer;
    allocation->next = context->allocations;
    context->allocations = allocation;
}

static AsmAllocation **find_allocation(void *pointer) {
    AsmErrorContext *context = asm_error_context();
    if (context == NULL) {
        return NULL;
    }

    AsmAllocation **allocation = &context->allocations;
    while (*allocation != NULL && (*allocation)->pointer != pointer) {
        allocation = &(*allocation)->next;
    }
    return allocation;
}

void *asm_malloc(size_t size) {
    void *result = malloc(size == 0 ? 1 : size);
    if (result == NULL) {
        asm_fail(ZLASM_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
    }
    track(result);
    return result;
}

void *asm_calloc(size_t count, size_t size) {
    void *result = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (result == NULL) {
        asm_fail(ZLASM_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
    }
    track(result);
    return result;
}

void *asm_realloc(void *pointer, size_t size) {
    AsmAllocation **allocation = find_allocation(pointer);
    void *result = realloc(pointer, size == 0 ? 1 : size);
    if (result == NULL) {
        asm_fail(ZLASM_DIAGNOSTIC_OUT_OF_MEMORY, "Out of memory");
    }
    if (allocation != NULL && *allocation != NULL) {
        (*allocation)->pointer = result;
    } else {
        track(result);
    }
    return result;
}

char *asm_strdup(const char *string) {
    if (string == NULL) {
        asm_fail(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Cannot duplicate a null string");
    }

    size_t size = strlen(string) + 1;
    char *result = asm_malloc(size);
    memcpy(result, string, size);
    return result;
}

void asm_memory_detach(void *pointer) {
    AsmAllocation **allocation = find_allocation(pointer);
    if (allocation != NULL && *allocation != NULL) {
        AsmAllocation *removed = *allocation;
        *allocation = removed->next;
        free(removed);
    }
}

void asm_free(void *pointer) {
    asm_memory_detach(pointer);
    free(pointer);
}

void asm_memory_release_all(void) {
    AsmErrorContext *context = asm_error_context();
    if (context == NULL) {
        return;
    }

    while (context->allocations != NULL) {
        AsmAllocation *allocation = context->allocations;
        context->allocations = allocation->next;
        free(allocation->pointer);
        free(allocation);
    }
}

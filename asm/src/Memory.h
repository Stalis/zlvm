#ifndef ZLVM_ASM_MEMORY_H
#define ZLVM_ASM_MEMORY_H

#include "Error.h"

void *asm_malloc(size_t size);
void *asm_calloc(size_t count, size_t size);
void *asm_realloc(void *pointer, size_t size);
char *asm_strdup(const char *string);
void asm_free(void *pointer);
void asm_memory_detach(void *pointer);
void asm_memory_release_all(void);

#endif // ZLVM_ASM_MEMORY_H

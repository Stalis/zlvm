#ifndef ZLVM_EMULATOR_ERROR_H
#define ZLVM_EMULATOR_ERROR_H

#include "Types.h"

#include <stdio.h>
#include <stdlib.h>

#define ZLVM_CRASH(message) vm_crash((message), __FILE__, __LINE__)

static inline _Noreturn void vm_crash(const char* message, const char* source_file,
                                      size_t source_line) {
    fprintf(stderr, "Crash at %s:%zu: %s\n", source_file, source_line, message);
    exit(EXIT_FAILURE);
}

#endif // ZLVM_EMULATOR_ERROR_H

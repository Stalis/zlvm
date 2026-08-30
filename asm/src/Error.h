#ifndef ZLVM_ASM_ERROR_H
#define ZLVM_ASM_ERROR_H

#include "Token.h"
#include "asm/zlasm.h"

#include <setjmp.h>

#define ZLASM_FAIL(code, message) asm_fail((code), (message))
#define ZLASM_TOKEN_FAIL(code, message, token) asm_token_fail((code), (message), (token))

typedef struct AsmAllocation AsmAllocation;

typedef struct AsmErrorContext {
    jmp_buf jump;
    ZlasmDiagnostic *diagnostic;
    const char *source_filename;
    AsmAllocation *allocations;
} AsmErrorContext;

void asm_error_begin(AsmErrorContext *context, ZlasmDiagnostic *diagnostic,
                     const char *source_filename);
void asm_error_end(void);
AsmErrorContext *asm_error_context(void);
_Noreturn void asm_fail(ZlasmDiagnosticCode code, const char *message);
_Noreturn void asm_token_fail(ZlasmDiagnosticCode code, const char *message, const Token *token);

#endif // ZLVM_ASM_ERROR_H

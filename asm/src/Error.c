#include "Error.h"

#include <stdio.h>
#include <stdlib.h>

static _Thread_local AsmErrorContext *current_context;

void asm_error_begin(AsmErrorContext *context, ZlasmDiagnostic *diagnostic,
                     const char *source_filename) {
    context->diagnostic = diagnostic;
    context->source_filename = source_filename == NULL ? "<memory>" : source_filename;
    context->allocations = NULL;
    current_context = context;
}

void asm_error_end(void) {
    current_context = NULL;
}

AsmErrorContext *asm_error_context(void) {
    return current_context;
}

_Noreturn void asm_fail(ZlasmDiagnosticCode code, const char *message) {
    if (current_context == NULL) {
        fprintf(stderr, "Assembler error: %s\n", message);
        exit(EXIT_FAILURE);
    }

    current_context->diagnostic->code = code;
    current_context->diagnostic->message = message;
    current_context->diagnostic->source_filename = current_context->source_filename;
    longjmp(current_context->jump, 1);
}

_Noreturn void asm_token_fail(ZlasmDiagnosticCode code, const char *message, const Token *token) {
    if (token == NULL) {
        asm_fail(code, message);
    }

    if (current_context == NULL) {
        fprintf(stderr, "Assembler error at %zu:%zu: %s\n", token->line, token->col, message);
        exit(EXIT_FAILURE);
    }

    current_context->diagnostic->line = token->line;
    current_context->diagnostic->column = token->col;
    current_context->diagnostic->byte_offset = token->pos;
    current_context->diagnostic->byte_length = token->source_size;
    current_context->diagnostic->has_source_location = true;
    asm_fail(code, message);
}

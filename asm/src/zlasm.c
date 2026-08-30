//
// Created by Stanislav on 2019-06-04.
//

#include "zlasm.h"

#include <setjmp.h>
#include <stdlib.h>

#include "Error.h"
#include "Memory.h"
#include <Assembler.h>
#include <Lexer.h>
#include <Parser.h>

ZlasmResult zlasm_assemble(const char *source, const char *source_filename) {
    ZlasmResult result = {0};
    AsmErrorContext *error = calloc(1, sizeof *error);
    ZlasmDiagnostic *diagnostic = calloc(1, sizeof *diagnostic);
    if (error == NULL || diagnostic == NULL) {
        free(error);
        free(diagnostic);
        result.diagnostic.code = ZLASM_DIAGNOSTIC_OUT_OF_MEMORY;
        result.diagnostic.message = "Out of memory";
        result.diagnostic.source_filename = source_filename == NULL ? "<memory>" : source_filename;
        return result;
    }

    if (source == NULL) {
        diagnostic->code = ZLASM_DIAGNOSTIC_INVALID_ARGUMENT;
        diagnostic->message = "Source is null";
        diagnostic->source_filename = source_filename == NULL ? "<memory>" : source_filename;
        result.diagnostic = *diagnostic;
        free(diagnostic);
        free(error);
        return result;
    }

    asm_error_begin(error, diagnostic, source_filename);
    if (setjmp(error->jump) != 0) {
        asm_memory_release_all();
        asm_error_end();
        result.diagnostic = *diagnostic;
        free(diagnostic);
        free(error);
        return result;
    }

    LexerState *lexer = asm_malloc(sizeof(LexerState));
    lexer_init(lexer, (char *)source);

    Token *token = lexer_readToken(lexer);
    while (token != NULL) {
        token = lexer_readToken(lexer);
    }

    ParserContext *parser = asm_malloc(sizeof(ParserContext));
    parser_init(parser);
    parser_parse(parser, tokenStream_new(lexer->_tokens));
    asm_free(lexer);

    AssemblerContext *assembler = asm_malloc(sizeof(AssemblerContext));
    asm_init(assembler);
    asm_processDirectives(assembler, parser);
    asm_free(parser);
    asm_processLabels(assembler);
    result.binary = asm_translate(assembler, &result.binary_size);

    asm_free(assembler);

    asm_memory_detach(result.binary);
    asm_memory_release_all();
    asm_error_end();
    free(diagnostic);
    free(error);
    return result;
}

void zlasm_result_free(ZlasmResult *result) {
    if (result != NULL) {
        free(result->binary);
        *result = (ZlasmResult){0};
    }
}

byte *assemblySource(char *source, size_t *binary_size) {
    if (binary_size == NULL) {
        return NULL;
    }

    ZlasmResult result = zlasm_assemble(source, NULL);
    *binary_size = result.binary_size;
    return result.binary;
}

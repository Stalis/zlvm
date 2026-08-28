#include "Parser.h"

#include "Memory.h"

static Token *read_token(TokenStream *stream) {
    if (tokenStream_isEof(stream)) {
        return NULL;
    }
    return tokenStream_read(stream);
}

static void append_token(TokenList **first, TokenList **last, Token *token) {
    TokenList *item = asm_calloc(1, sizeof *item);
    item->value = token;

    if (*first == NULL) {
        *first = item;
    } else {
        (*last)->next = item;
    }
    *last = item;
}

void parser_init(ParserContext *context) {
    context->lines = asm_calloc(1, sizeof *context->lines);
    line_list_init(context->lines);
    context->lines_count = 0;
}

static TokenStream *parser_get_line(TokenStream *stream) {
    Token *current = read_token(stream);
    while (current != NULL && (current->type == TOK_COMMENT || current->type == TOK_NEWLINE)) {
        current = read_token(stream);
    }

    if (current == NULL) {
        return NULL;
    }

    TokenList *first = NULL;
    TokenList *last = NULL;
    append_token(&first, &last, current);
    current = read_token(stream);

    if (first->value->type == TOK_LABEL_INIT && current != NULL && current->type == TOK_NEWLINE) {
        current = read_token(stream);
    }

    while (current != NULL && current->type != TOK_NEWLINE) {
        if (current->type == TOK_COMMENT) {
            break;
        }
        if (current->type != TOK_COMMA) {
            append_token(&first, &last, current);
        }
        current = read_token(stream);
    }

    return tokenStream_new(first);
}

static Statement *parser_read_statement(TokenStream *stream, Token *current) {
    Statement *statement = asm_malloc(sizeof *statement);
    statement_init(statement);
    statement->opcode = current;

    current = read_token(stream);
    if (current != NULL && current->type == TOK_ID) {
        statement->cond = current;
        current = read_token(stream);
    }

    if (current != NULL && current->type == TOK_REGISTER) {
        statement->reg1 = current;
        current = read_token(stream);
        if (current != NULL && current->type == TOK_REGISTER) {
            statement->reg2 = current;
            current = read_token(stream);
        }
    }

    if (current != NULL && (current->type == TOK_LABEL_USE || current->type == TOK_CHAR_LITERAL ||
                            current->type == TOK_INT_BIN || current->type == TOK_INT_OCT ||
                            current->type == TOK_INT_DEC || current->type == TOK_INT_HEX)) {
        statement->imm = current;
        current = read_token(stream);
    }

    if (current != NULL) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN, "Unexpected trailing token", current);
    }

    return statement;
}

static Directive *parser_read_directive(TokenStream *stream, Token *current) {
    Directive *directive = asm_malloc(sizeof *directive);
    if (!directive_init(directive, current)) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_DIRECTIVE, "Unknown directive", current);
    }

    for (current = read_token(stream); current != NULL; current = read_token(stream)) {
        directive_add_arg(directive, current);
    }
    return directive;
}

static Line *parser_read_line(TokenStream *stream) {
    Line *line = asm_calloc(1, sizeof *line);
    Token *current = read_token(stream);

    if (current != NULL && current->type == TOK_LABEL_INIT) {
        Token *label = current;
        line->label = current->value;
        current = read_token(stream);
        if (current == NULL) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN,
                             "Label is not followed by a statement or directive", label);
        }
    }

    if (current == NULL) {
        ZLASM_FAIL(ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN, "Expected a statement or directive");
    }

    switch (current->type) {
        case TOK_ID:
            line->type = L_STMT;
            line->stmt = parser_read_statement(stream, current);
            break;
        case TOK_DIRECTIVE:
            line->type = L_DIR;
            line->dir = parser_read_directive(stream, current);
            break;
        default:
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNEXPECTED_TOKEN, "Unexpected token", current);
    }

    return line;
}

void parser_addLine(ParserContext *context, Line *line) {
    line_list_add(context->lines, line);
    context->lines_count++;
}

void parser_parse(ParserContext *context, TokenStream *stream) {
    TokenStream *line_stream = parser_get_line(stream);
    while (line_stream != NULL) {
        parser_addLine(context, parser_read_line(line_stream));
        asm_free(line_stream);
        line_stream = parser_get_line(stream);
    }
    asm_free(stream);
}

void parser_clear(ParserContext *context) {
    line_list_free(context->lines);
    context->lines = NULL;
    context->lines_count = 0;
}

#include "Token.h"

#include "Memory.h"

#include <stdio.h>
#include <string.h>

static const char* get_token_type_name(TokenType type);

void token_print(Token* token) {
    const char* format = token->type == TOK_NEWLINE ? "(%4zu:%3zu:%2zu) %12s[%2zu]\n"
                                                    : "(%4zu:%3zu:%2zu) %12s[%2zu] %s\n";
    if (token->type == TOK_NEWLINE) {
        fprintf(stdout, format, token->pos, token->line, token->col,
                get_token_type_name(token->type), token->size);
    } else {
        fprintf(stdout, format, token->pos, token->line, token->col,
                get_token_type_name(token->type), token->size, token->value);
    }
}

void token_free(Token* token) {
    if (token == NULL) {
        return;
    }
    asm_free(token->value);
    asm_free(token);
}

static const char* get_token_type_name(TokenType type) {
    switch (type) {
        case TOK_COMMENT:
            return "COMMENT";
        case TOK_NEWLINE:
            return "NEWLINE";
        case TOK_ID:
            return "ID";
        case TOK_REGISTER:
            return "REGISTER";
        case TOK_LABEL_INIT:
            return "LABEL_INIT";
        case TOK_LABEL_USE:
            return "LABEL_USE";
        case TOK_DIRECTIVE:
            return "DIRECTIVE";
        case TOK_STRING_LITERAL:
            return "STRING_LIT";
        case TOK_CHAR_LITERAL:
            return "CHAR_LIT";
        case TOK_INT_HEX:
            return "INT_HEX";
        case TOK_INT_DEC:
            return "INT_DEC";
        case TOK_INT_OCT:
            return "INT_OCT";
        case TOK_INT_BIN:
            return "INT_BIN";
        case TOK_COMMA:
            return "COMMA";
        default:
            return "UNKNOWN";
    }
}

dword token_get_int_value(Token* token) {
    switch (token->type) {
        case TOK_INT_HEX:
            return strtoull(token->value, NULL, 16);
        case TOK_INT_DEC:
            return strtoull(token->value, NULL, 10);
        case TOK_INT_OCT:
            return strtoull(token->value, NULL, 8);
        case TOK_INT_BIN:
            return strtoull(token->value, NULL, 2);
        default:
            ZLASM_TOKEN_CRASH("Token is not an integer", token);
    }
}

char token_get_char_value(char* value) {
    if (value[0] != '\\') {
        return value[0];
    }

    switch (value[1]) {
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'a':
            return '\a';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case 'b':
            return '\b';
        case '\\':
            return '\\';
        default:
            ZLASM_CRASH("Invalid escape character");
    }
}

byte* token_get_string_value(char* value, size_t* output_size) {
    *output_size = strlen(value);
    byte* result = asm_malloc(*output_size);
    memcpy(result, value, *output_size);
    return result;
}

byte* token_get_raw_data(Token* token, size_t* output_size) {
    switch (token->type) {
        case TOK_STRING_LITERAL:
            return token_get_string_value(token->value, output_size);
        case TOK_CHAR_LITERAL: {
            *output_size = sizeof(byte);
            byte* result = asm_malloc(*output_size);
            *result = (byte)token_get_char_value(token->value);
            return result;
        }
        case TOK_INT_HEX:
        case TOK_INT_DEC:
        case TOK_INT_OCT:
        case TOK_INT_BIN: {
            dword value = token_get_int_value(token);
            *output_size = sizeof value;
            byte* result = asm_malloc(*output_size);
            memcpy(result, &value, *output_size);
            return result;
        }
        default:
            ZLASM_TOKEN_CRASH("Token cannot be emitted as raw data", token);
    }
}

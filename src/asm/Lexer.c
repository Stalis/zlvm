//
// Created by Stanislav on 2019-05-07.
//

#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include "Lexer.h"

static const char NEWLINE = '\n';
static const char COMMENT_MARK = ';';
static const char LABEL_INIT_MARK = ':';
static const char LABEL_USE_MARK = '#';
static const char REGISTER_MARK = '$';
static const char DIRECTIVE_MARK = '.';
static const char STRING_QUOTE = '"';
static const char CHAR_QUOTE = '\'';
static const char DIGIT_DELIMITER = '_';

static inline bool is_ignored_char(char);
static inline bool is_id_char(char);
static inline bool is_hex_char(char);
static inline bool is_dec_char(char);
static inline bool is_oct_char(char);
static inline bool is_bin_char(char);

static void remove_digit_delimiters(char*, size_t);

static void tokenlist_add(struct TokenList*, struct Token*);
static void tokenlist_free(struct TokenList*);

void lexer_init(struct LexerState* state, char* source) {
    state->_tokens = (struct TokenList*) malloc(sizeof(struct TokenList));
    state->_tokens->value = NULL;
    state->_tokens->next = NULL;
    state->ptr = source;
}

struct Token* lexer_read_token(struct LexerState* state) {
    char* right = state->ptr;
    char* first;
    char* last = NULL;
    size_t value_size;
    char c = *(right);
    enum TokenType type;
    struct Token* result;

    while (is_ignored_char(c)) {
        c = *(++right);

        if (c == EOF || c == '\0') {
            return NULL;
        }
    }

    first = right;

    switch (c) {
        case NEWLINE:
            c = *(++right);
            type = TOK_NEWLINE;
            break;

        case COMMENT_MARK:
            c = *(++right);
            first = right;
            while (c != NEWLINE) c = *(++right);
            type = TOK_COMMENT;
            break;

        case LABEL_INIT_MARK:
            c = *(++right);
            first = right;
            while (!is_ignored_char(c) && c != NEWLINE) c = *(++right);
            type = TOK_LABEL_INIT;
            break;

        case LABEL_USE_MARK:
            c = *(++right);
            first = right;
            while (!is_ignored_char(c) && c != NEWLINE) c = *(++right);
            type = TOK_LABEL_USE;
            break;

        case REGISTER_MARK:
            c = *(++right);
            first = right;
            while (!is_ignored_char(c) && c != NEWLINE) c = *(++right);
            type = TOK_REGISTER;
            break;

        case DIRECTIVE_MARK:
            c = *(++right);
            first = right;
            while (!is_ignored_char(c) && c != NEWLINE) c = *(++right);
            type = TOK_DIRECTIVE;
            break;

        case STRING_QUOTE:
            c = *(++right);
            first = right;
            while (c != STRING_QUOTE) c = *(++right);
            type = TOK_STRING_LITERAL;
            last = right++;
            break;

        case CHAR_QUOTE:
            c = *(++right);
            first = right;
            while (c != CHAR_QUOTE) c = *(++right);
            type = TOK_CHAR_LITERAL;
            last = right++;
            break;

        default: {
            if (is_dec_char(c)) {
                if (c == '0') {
                    char mark = *(right + 1);
                    switch (mark) {
                        case 'X':
                        case 'x':
                            right += 2;
                            first = right;
                            while (is_hex_char(c)) c = *(++right);
                            type = TOK_INT_HEX;
                            break;

                        case 'O':
                        case 'o':
                            right += 2;
                            first = right;
                            while (is_oct_char(c)) c = *(++right);
                            type = TOK_INT_OCT;
                            break;

                        case 'B':
                        case 'b':
                            right += 2;
                            first = right;
                            while (is_bin_char(c)) c = *(++right);
                            type = TOK_INT_BIN;
                            break;

                        default:
                            goto __parse_int_dec;
                    }
                } else {
                    __parse_int_dec:
                    while (is_dec_char(c)) c = *(++right);
                    type = TOK_INT_DEC;
                    break;
                }
            } else {
                while (!is_ignored_char(c) && c != NEWLINE) c = *(++right);
                type = TOK_ID;
            }
        }
            break;
    }

    if (last == NULL) last = right;
    value_size = last - first;

    result = (struct Token*) malloc(sizeof(struct Token));
    result->size = value_size;
    result->type = type;
    result->value = (char*) calloc(value_size + 1, sizeof(char));
    strncpy(result->value, first, value_size);

    switch (result->type) {
        case TOK_INT_HEX:
        case TOK_INT_DEC:
        case TOK_INT_OCT:
        case TOK_INT_BIN:
            remove_digit_delimiters(result->value, result->size);
            result->size = strlen(result->value);
            result->value = realloc(result->value, result->size + 1);
        default:
            break;
    }

    state->ptr = right;
    tokenlist_add(state->_tokens, result);
    return result;
}

static void tokenlist_add(struct TokenList* list, struct Token* t) {
    struct TokenList* buf = list;
    if (NULL == list->value) {
        list->value = t;
    } else {
        while (buf->next != NULL) {
            buf = buf->next;
        }
        buf->next = calloc(1, sizeof(struct TokenList));
        buf->next->value = t;
    }
}

static bool is_ignored_char(char c) {
    return ((isspace(c) != 0) && (c != NEWLINE)) || (c == EOF) || (c == '\0');
}

static bool is_id_char(char c) {
    return (isalpha(c) != 0 || c == DIGIT_DELIMITER);
}

static bool is_hex_char(char c) {
    return (isxdigit(c) != 0 || c == DIGIT_DELIMITER);
}

static bool is_dec_char(char c) {
    return (isdigit(c) != 0 || c == DIGIT_DELIMITER);
}

static bool is_oct_char(char c) {
    return ((c >= '0' && c <= '7') || c == DIGIT_DELIMITER);
}

static bool is_bin_char(char c) {
    return ((c >= '0' && c <= '1') || c == DIGIT_DELIMITER);
}

static void remove_digit_delimiters(char* string, size_t __size) {
    char* buf = (char*) calloc(__size, sizeof(char));
    size_t buf_i = 0;

    for (size_t i = 0; i < __size; i++) {
        if (string[i] != DIGIT_DELIMITER) {
            buf[buf_i++] = string[i];
        }
    }
    strncpy(string, buf, __size);
}

static void tokenlist_free(struct TokenList* list) {
    if (list->next != NULL) {
        tokenlist_free(list->next);
    }
    if (list->value != NULL) {
        token_free(list->value);
    }
    free(list);
}

void lexer_clear(struct LexerState* state) {
    tokenlist_free(state->_tokens);
}
//
// Created by Stanislav on 2019-05-07.
//

#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include "../include/Lexer.h"

static const char NEWLINE = '\n';
static const char COMMA = ',';
static const char COMMENT_MARK = ';';
static const char LABEL_INIT_MARK = ':';
static const char LABEL_USE_MARK = '#';
static const char REGISTER_MARK = '$';
static const char DIRECTIVE_MARK = '.';
static const char STRING_QUOTE = '"';
static const char CHAR_QUOTE = '\'';
static const char DIGIT_DELIMITER = '_';

static inline bool is_eof(char);
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
    state->source = source;
    state->pos = 0;
    state->line = 1;
    state->col = 1;
}

char lexer_peek_char(struct LexerState* l) {
    return l->source[l->pos];
}

char lexer_next_char(struct LexerState* l) {
    char val = l->source[++l->pos];
    if (val == '\n') {
        l->line++;
        l->col = 1;
    } else {
        l->col++;
    }
    return val;
}

char* lexer_ahead(struct LexerState* l) {
    return (l->source + l->pos);
}

struct Token* lexer_read_token(struct LexerState* state) {
#define right (lexer_ahead(state))

    char* first;
    char* last = NULL;
    size_t value_size;
    char c = *(right);
    enum TokenType type;
    struct Token* result;

    size_t pos = state->pos, line = state->line, col = state->col;


    while (is_ignored_char(c))
    {
        c = lexer_next_char(state);

        if (is_eof(c))
            return NULL;
    }

    first = right;
    c = lexer_peek_char(state);

    switch (c)
    {
        case NEWLINE:
            c = lexer_next_char(state);
            type = TOK_NEWLINE;
            break;

        case COMMA:
            c = lexer_next_char(state);
            type = TOK_COMMA;
            break;

        case COMMENT_MARK:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (c != NEWLINE) c = lexer_next_char(state);
            type = TOK_COMMENT;
            break;

        case LABEL_USE_MARK:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_next_char(state);
            type = TOK_LABEL_USE;
            break;

        case REGISTER_MARK:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_next_char(state);
            type = TOK_REGISTER;
            break;

        case DIRECTIVE_MARK:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_next_char(state);
            type = TOK_DIRECTIVE;
            break;

        case STRING_QUOTE:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (c != STRING_QUOTE) c = lexer_next_char(state);
            type = TOK_STRING_LITERAL;
            last = state->source + (state->pos++);
            break;

        case CHAR_QUOTE:
            c = lexer_next_char(state);
            first = state->source + state->pos;
            while (c != CHAR_QUOTE) c = lexer_next_char(state);
            type = TOK_CHAR_LITERAL;
            last = state->source + (state->pos++);
            break;

        default:
        {
            if (is_dec_char(c))
            {
                if (c == '0')
                {
                    char mark = *(right + 1);
                    switch (mark)
                    {
                        case 'X':
                        case 'x':
                        case 'O':
                        case 'o':
                        case 'B':
                        case 'b':
                            lexer_next_char(state); lexer_next_char(state);
                            first = right;
                            if (mark == 'X' || mark == 'x') {
                                while (is_hex_char(c)) c = lexer_next_char(state);
                                type = TOK_INT_HEX;
                            } else if (mark == 'O' || mark == 'o') {
                                while (is_oct_char(c)) c = lexer_next_char(state);
                                type = TOK_INT_OCT;
                            } else {
                                while (is_bin_char(c)) c = lexer_next_char(state);
                                type = TOK_INT_BIN;
                            }
                            break;

                        default:
                            goto __parse_int_dec;
                    }
                }
                else
                {
                __parse_int_dec:
                    while (is_dec_char(c)) c = lexer_next_char(state);
                    type = TOK_INT_DEC;
                    break;
                }
            }
            else
            {
                while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_next_char(state);
                if (*(right - 1) == LABEL_INIT_MARK)
                {
                    last = right - 1;
                    type = TOK_LABEL_INIT;
                }
                else
                {
                    type = TOK_ID;
                }
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

    result->pos = pos;
    result->line = line;
    result->col = col;

    switch (result->type)
    {
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

    tokenlist_add(state->_tokens, result);
    return result;
#undef right
}

static void tokenlist_add(struct TokenList* list, struct Token* t) {
    struct TokenList* buf = list;
    if (NULL == list->value)
    {
        list->value = t;
    }
    else
    {
        while (buf->next != NULL)
        {
            buf = buf->next;
        }
        buf->next = calloc(1, sizeof(struct TokenList));
        buf->next->value = t;
    }
}

static inline bool is_eof(char c) {
    return (c == EOF) || (c == '\0');
}

static inline bool is_ignored_char(char c) {
    return ((isspace(c) != 0) && (c != NEWLINE)) || (c == EOF) || (c == '\0');
}

static inline bool is_id_char(char c) {
    return (isalpha(c) != 0 || c == DIGIT_DELIMITER);
}

static inline bool is_hex_char(char c) {
    return (isxdigit(c) != 0 || c == DIGIT_DELIMITER);
}

static inline bool is_dec_char(char c) {
    return (isdigit(c) != 0 || c == DIGIT_DELIMITER);
}

static inline bool is_oct_char(char c) {
    return ((c >= '0' && c <= '7') || c == DIGIT_DELIMITER);
}

static inline bool is_bin_char(char c) {
    return ((c >= '0' && c <= '1') || c == DIGIT_DELIMITER);
}

static void remove_digit_delimiters(char* string, size_t __size) {
    char* buf = (char*) calloc(__size, sizeof(char));
    size_t buf_i = 0;

    for (size_t i = 0; i < __size; i++)
    {
        if (string[i] != DIGIT_DELIMITER)
        {
            buf[buf_i++] = string[i];
        }
    }
    strncpy(string, buf, __size);
}

static void tokenlist_free(struct TokenList* list) {
    if (list->next != NULL)
    {
        tokenlist_free(list->next);
    }
    if (list->value != NULL)
    {
        token_free(list->value);
    }
    free(list);
}

void lexer_clear(struct LexerState* state) {
    tokenlist_free(state->_tokens);
}
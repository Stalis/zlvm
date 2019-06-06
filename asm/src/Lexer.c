// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-07.
//

#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

#include "Lexer.h"
#include "Memory.h"

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

static inline void remove_digit_delimiters(char*, size_t);

static void tokenlist_add(struct TokenList*, struct Token*);
static void tokenlist_free(struct TokenList*);

struct TokenStream* tokenStream_new(struct TokenList* l) {
    struct TokenStream* res = malloc(sizeof(struct TokenStream));
    res->_first = l;
    return res;
}

struct Token* tokenStream_read(TokenStream* stream) {
    if (NULL == stream->_first)
    {
        return NULL;
    }
    struct Token* buf = stream->_first->value;
    stream->_first->value = NULL;
    stream->_first = stream->_first->next;
    return buf;
}

bool tokenStream_isEof(TokenStream* stream) {
    return NULL == stream->_first || NULL == stream->_first->value;
}

struct Token* tokenStream_peek(struct TokenStream* stream) {
    if (NULL == stream->_first)
    {
        return NULL;
    }
    return stream->_first->value;
}

void lexer_init(struct LexerState* state, char* source) {
    state->_tokens = (struct TokenList*) malloc(sizeof(struct TokenList));
    state->_tokens->value = NULL;
    state->_tokens->next = NULL;
    state->source = source;
    state->_len = strlen(source);
    state->pos = 0;
    state->line = 1;
    state->col = 1;
}

char lexer_peekChar(struct LexerState* s) {
    return s->source[s->pos];
}

char lexer_nextChar(struct LexerState* s) {
    if (s->pos >= s->_len)
        return 0;

    char val = s->source[++s->pos];
    if (val == '\n') {
        s->line++;
        s->col = 1;
    } else {
        s->col++;
    }
    return val;
}

char* lexer_ahead(struct LexerState* l) {
    return (l->source + l->pos);
}

struct Token* lexer_readToken(struct LexerState* s) {
#define right (lexer_ahead(s))

    char* first;
    char* last = NULL;
    size_t value_size;
    char c = *(right);
    enum TokenType type;
    struct Token* result;

    size_t pos = s->pos, line = s->line, col = s->col;


    while (is_ignored_char(c))
    {
        c = lexer_nextChar(s);

        if (is_eof(c))
            return NULL;
    }

    first = right;
    c = lexer_peekChar(s);

    switch (c)
    {
        case NEWLINE:
            c = lexer_nextChar(s);
            type = TOK_NEWLINE;
            break;

        case COMMA:
            c = lexer_nextChar(s);
            type = TOK_COMMA;
            break;

        case COMMENT_MARK:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (c != NEWLINE && !is_eof(c)) c = lexer_nextChar(s);
            type = TOK_COMMENT;
            break;

        case LABEL_USE_MARK:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_nextChar(s);
            type = TOK_LABEL_USE;
            break;

        case REGISTER_MARK:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_nextChar(s);
            type = TOK_REGISTER;
            break;

        case DIRECTIVE_MARK:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_nextChar(s);
            type = TOK_DIRECTIVE;
            break;

        case STRING_QUOTE:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (c != STRING_QUOTE) c = lexer_nextChar(s);
            type = TOK_STRING_LITERAL;
            last = s->source + (s->pos++);
            break;

        case CHAR_QUOTE:
            c = lexer_nextChar(s);
            first = s->source + s->pos;
            while (c != CHAR_QUOTE) c = lexer_nextChar(s);
            type = TOK_CHAR_LITERAL;
            last = s->source + (s->pos++);
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
                            lexer_nextChar(s);
                            lexer_nextChar(s);
                            first = right;
                            if (mark == 'X' || mark == 'x') {
                                while (is_hex_char(c)) c = lexer_nextChar(s);
                                type = TOK_INT_HEX;
                            } else if (mark == 'O' || mark == 'o') {
                                while (is_oct_char(c)) c = lexer_nextChar(s);
                                type = TOK_INT_OCT;
                            } else {
                                while (is_bin_char(c)) c = lexer_nextChar(s);
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
                    while (is_dec_char(c)) c = lexer_nextChar(s);
                    type = TOK_INT_DEC;
                    break;
                }
            }
            else
            {
                while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) c = lexer_nextChar(s);
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

    tokenlist_add(s->_tokens, result);
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
    return (c == (char) EOF) || (c == '\0');
}

static inline bool is_ignored_char(char c) {
    return ((isspace(c) != 0) && (c != NEWLINE)) || is_eof(c);
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

static inline void remove_digit_delimiters(char* string, size_t __size) {
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
    free(buf);
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
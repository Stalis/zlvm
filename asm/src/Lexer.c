// Created by Stanislav on 2019-05-07.
//

#include "Lexer.h"

#include "Memory.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum {
    NEWLINE = '\n',
    COMMA = ',',
    COMMENT_MARK = ';',
    LABEL_INIT_MARK = ':',
    LABEL_USE_MARK = '#',
    REGISTER_MARK = '$',
    DIRECTIVE_MARK = '.',
    STRING_QUOTE = '"',
    CHAR_QUOTE = '\'',
    DIGIT_DELIMITER = '_',
};

static bool is_eof(char character);
static bool is_ignored_char(char character);
static bool is_dec_char(char character);
static bool is_token_end(char character);
static bool is_digit_for_base(char character, int base);

static void remove_digit_delimiters(char *string, size_t size);
static void validate_numeric_token(Token *token, const char *digits, size_t digits_size, int base);
static void validate_numeric_value(Token *token);

static void token_list_add(struct TokenList *, struct Token *);
static void token_list_free(struct TokenList *);

TokenStream *tokenStream_new(TokenList *list) {
    TokenStream *stream = asm_malloc(sizeof *stream);
    stream->_first = list;
    return stream;
}

Token *tokenStream_read(TokenStream *stream) {
    if (stream->_first == NULL) {
        return NULL;
    }
    Token *token = stream->_first->value;
    stream->_first->value = NULL;
    stream->_first = stream->_first->next;
    return token;
}

bool tokenStream_isEof(TokenStream *stream) {
    return stream->_first == NULL || stream->_first->value == NULL;
}

Token *tokenStream_peek(TokenStream *stream) {
    if (stream->_first == NULL) {
        return NULL;
    }
    return stream->_first->value;
}

void lexer_init(LexerState *state, char *source) {
    state->_tokens = asm_calloc(1, sizeof *state->_tokens);
    state->source = source;
    state->_len = strlen(source);
    state->pos = 0;
    state->line = 1;
    state->col = 1;
}

char lexer_peekChar(LexerState *state) {
    return state->source[state->pos];
}

char lexer_nextChar(LexerState *state) {
    if (state->pos >= state->_len) {
        return 0;
    }

    char value = state->source[state->pos++];
    if (value == '\n') {
        state->line++;
        state->col = 1;
    } else {
        state->col++;
    }
    return state->source[state->pos];
}

char *lexer_ahead(LexerState *state) {
    return state->source + state->pos;
}

Token *lexer_readToken(LexerState *state) {
#define CURRENT (lexer_ahead(state))

    char *first;
    char *last = NULL;
    size_t value_size;
    char c = *CURRENT;
    enum TokenType type;
    struct Token *result;

    while (is_ignored_char(c)) {
        c = lexer_nextChar(state);

        if (is_eof(c)) {
            return NULL;
        }
    }

    size_t position = state->pos;
    size_t line = state->line;
    size_t column = state->col;

    first = CURRENT;
    c = lexer_peekChar(state);

    switch (c) {
        case NEWLINE:
            c = lexer_nextChar(state);
            type = TOK_NEWLINE;
            break;

        case COMMA:
            c = lexer_nextChar(state);
            type = TOK_COMMA;
            break;

        case COMMENT_MARK:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (c != NEWLINE && !is_eof(c)) {
                c = lexer_nextChar(state);
            }
            type = TOK_COMMENT;
            break;

        case LABEL_USE_MARK:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) {
                c = lexer_nextChar(state);
            }
            type = TOK_LABEL_USE;
            break;

        case REGISTER_MARK:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) {
                c = lexer_nextChar(state);
            }
            type = TOK_REGISTER;
            break;

        case DIRECTIVE_MARK:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) {
                c = lexer_nextChar(state);
            }
            type = TOK_DIRECTIVE;
            break;

        case STRING_QUOTE:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (c != STRING_QUOTE) {
                if (is_eof(c)) {
                    Token token = {.pos = position,
                                   .line = line,
                                   .col = column,
                                   .source_size = state->pos - position};
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNTERMINATED_LITERAL,
                                     "Unterminated string literal", &token);
                }
                c = lexer_nextChar(state);
            }
            type = TOK_STRING_LITERAL;
            last = state->source + state->pos;
            lexer_nextChar(state);
            break;

        case CHAR_QUOTE:
            c = lexer_nextChar(state);
            first = state->source + state->pos;
            while (c != CHAR_QUOTE) {
                if (is_eof(c)) {
                    Token token = {.pos = position,
                                   .line = line,
                                   .col = column,
                                   .source_size = state->pos - position};
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNTERMINATED_LITERAL,
                                     "Unterminated character literal", &token);
                }
                c = lexer_nextChar(state);
            }
            type = TOK_CHAR_LITERAL;
            last = state->source + state->pos;
            lexer_nextChar(state);
            break;

        default: {
            if (is_dec_char(c)) {
                char prefix = c == '0' ? CURRENT[1] : '\0';
                bool has_prefix = prefix == 'x' || prefix == 'X' || prefix == 'o' ||
                                  prefix == 'O' || prefix == 'b' || prefix == 'B';
                if (has_prefix) {
                    lexer_nextChar(state);
                    lexer_nextChar(state);
                    first = CURRENT;
                    c = lexer_peekChar(state);

                    if (prefix == 'x' || prefix == 'X') {
                        while (!is_token_end(c)) {
                            c = lexer_nextChar(state);
                        }
                        type = TOK_INT_HEX;
                    } else if (prefix == 'o' || prefix == 'O') {
                        while (!is_token_end(c)) {
                            c = lexer_nextChar(state);
                        }
                        type = TOK_INT_OCT;
                    } else {
                        while (!is_token_end(c)) {
                            c = lexer_nextChar(state);
                        }
                        type = TOK_INT_BIN;
                    }
                } else {
                    while (!is_token_end(c)) {
                        c = lexer_nextChar(state);
                    }
                    type = TOK_INT_DEC;
                }
            } else {
                while (!is_ignored_char(c) && c != NEWLINE && c != COMMA) {
                    c = lexer_nextChar(state);
                }
                if (*(CURRENT - 1) == LABEL_INIT_MARK) {
                    last = CURRENT - 1;
                    type = TOK_LABEL_INIT;
                } else {
                    type = TOK_ID;
                }
            }
        } break;
    }

    if (last == NULL) {
        last = CURRENT;
    }
    value_size = (size_t)(last - first);

    result = asm_malloc(sizeof *result);
    result->size = value_size;
    result->type = type;
    result->value = asm_calloc(value_size + 1, sizeof *result->value);
    memcpy(result->value, first, value_size);

    result->pos = position;
    result->line = line;
    result->col = column;
    result->source_size = state->pos - position;

    switch (result->type) {
        case TOK_INT_HEX:
            validate_numeric_token(result, first, value_size, 16);
            remove_digit_delimiters(result->value, result->size);
            result->size = strlen(result->value);
            result->value = asm_realloc(result->value, result->size + 1);
            validate_numeric_value(result);
            break;
        case TOK_INT_DEC:
            validate_numeric_token(result, first, value_size, 10);
            remove_digit_delimiters(result->value, result->size);
            result->size = strlen(result->value);
            result->value = asm_realloc(result->value, result->size + 1);
            validate_numeric_value(result);
            break;
        case TOK_INT_OCT:
            validate_numeric_token(result, first, value_size, 8);
            remove_digit_delimiters(result->value, result->size);
            result->size = strlen(result->value);
            result->value = asm_realloc(result->value, result->size + 1);
            validate_numeric_value(result);
            break;
        case TOK_INT_BIN:
            validate_numeric_token(result, first, value_size, 2);
            remove_digit_delimiters(result->value, result->size);
            result->size = strlen(result->value);
            result->value = asm_realloc(result->value, result->size + 1);
            validate_numeric_value(result);
            break;
        default:
            break;
    }

    token_list_add(state->_tokens, result);
    return result;
#undef CURRENT
}

static void token_list_add(TokenList *list, Token *token) {
    TokenList *last = list;
    if (list->value == NULL) {
        list->value = token;
    } else {
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = asm_calloc(1, sizeof *last->next);
        last->next->value = token;
    }
}

static bool is_eof(char character) {
    return character == '\0';
}

static bool is_ignored_char(char character) {
    return (isspace((unsigned char)character) != 0 && character != NEWLINE) || is_eof(character);
}

static bool is_dec_char(char character) {
    return isdigit((unsigned char)character) != 0 || character == DIGIT_DELIMITER;
}

static bool is_token_end(char character) {
    return is_ignored_char(character) || character == NEWLINE || character == COMMA ||
           character == COMMENT_MARK;
}

static bool is_digit_for_base(char character, int base) {
    if (character >= '0' && character <= '9') {
        return character - '0' < base;
    }
    character = (char)tolower((unsigned char)character);
    return base == 16 && character >= 'a' && character <= 'f';
}

static void validate_numeric_token(Token *token, const char *digits, size_t digits_size, int base) {
    if (digits_size == 0) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MALFORMED_NUMBER, "Malformed numeric literal", token);
    }

    for (size_t index = 0; index < digits_size; index++) {
        bool is_delimiter = digits[index] == DIGIT_DELIMITER;
        if ((!is_delimiter && !is_digit_for_base(digits[index], base)) ||
            (is_delimiter && (index == 0 || index + 1 == digits_size ||
                              !is_digit_for_base(digits[index - 1], base) ||
                              !is_digit_for_base(digits[index + 1], base)))) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MALFORMED_NUMBER, "Malformed numeric literal", token);
        }
    }
}

static void validate_numeric_value(Token *token) {
    (void)token_get_int_value(token);
}

static void remove_digit_delimiters(char *string, size_t size) {
    size_t output_index = 0;
    for (size_t input_index = 0; input_index < size; input_index++) {
        if (string[input_index] != DIGIT_DELIMITER) {
            string[output_index++] = string[input_index];
        }
    }
    string[output_index] = '\0';
}

static void token_list_free(struct TokenList *list) {
    if (list->next != NULL) {
        token_list_free(list->next);
    }
    if (list->value != NULL) {
        token_free(list->value);
    }
    asm_free(list);
}

void lexer_clear(LexerState *state) {
    if (state != NULL && state->_tokens != NULL) {
        token_list_free(state->_tokens);
        state->_tokens = NULL;
    }
}

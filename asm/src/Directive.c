#include "Directive.h"

#include "Memory.h"

#include <assert.h>
#include <string.h>

static DirectiveType get_directive_type(const Token *token);
static byte *encode_numeric_data(const Directive *directive, size_t element_size,
                                 dword maximum_value, size_t *output_size);

bool directive_init(Directive *directive, Token *name) {
    assert(directive != NULL);

    directive->type = get_directive_type(name);
    directive->name = name;
    directive->argc = 0;
    directive->argv = NULL;
    return directive->type != DIR_TOTAL;
}

void directive_add_arg(Directive *directive, Token *argument) {
    if (directive == NULL) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Directive is null", argument);
    }

    size_t new_count = directive->argc + 1;
    directive->argv = asm_realloc(directive->argv, new_count * sizeof *directive->argv);
    directive->argv[directive->argc] = argument;
    directive->argc = new_count;
}

void directive_free(Directive *directive) {
    if (directive == NULL) {
        return;
    }
    asm_free(directive->argv);
    asm_free(directive);
}

static DirectiveType get_directive_type(const Token *token) {
    static const struct {
        const char *name;
        DirectiveType type;
    } directives[] = {
        {"section", DIR_SECTION}, {"global", DIR_GLOBAL},   {"extern", DIR_EXTERN},
        {"align", DIR_ALIGN},     {"entry", DIR_ENTRY},     {"locate", DIR_LOCATE},
        {"ascii", DIR_ASCII},     {"asciiz", DIR_ASCIIZ},   {"byte", DIR_BYTE},
        {"hword", DIR_HWORD},     {"word", DIR_WORD},       {"dword", DIR_DWORD},
        {"space", DIR_SPACE},     {"macro", DIR_MACRO},     {"endmacro", DIR_ENDMACRO},
        {"proc", DIR_PROC},       {"endproc", DIR_ENDPROC},
    };

    for (size_t index = 0; index < sizeof directives / sizeof directives[0]; index++) {
        if (strcmp(token->value, directives[index].name) == 0) {
            return directives[index].type;
        }
    }
    return DIR_TOTAL;
}

byte *directive_get_raw_data(Directive *directive, size_t *output_size) {
    assert(directive != NULL);
    assert(output_size != NULL);
    assert(is_data_directive(directive->type));

    *output_size = 0;
    switch (directive->type) {
        case DIR_ASCII:
        case DIR_ASCIIZ: {
            byte *result = NULL;
            for (size_t index = 0; index < directive->argc; index++) {
                size_t value_size = 0;
                byte *value;
                if (directive->argv[index]->type == TOK_STRING_LITERAL) {
                    value = token_get_string_value(directive->argv[index]->value, &value_size);
                } else {
                    value_size = 1;
                    value = asm_malloc(value_size);
                    if (directive->argv[index]->type == TOK_CHAR_LITERAL) {
                        *value = (byte)token_get_char_value(directive->argv[index]);
                    } else {
                        dword numeric_value = token_get_int_value(directive->argv[index]);
                        if (numeric_value > BYTE_MAX) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                                             "Value does not fit in a byte",
                                             directive->argv[index]);
                        }
                        *value = (byte)numeric_value;
                    }
                }

                size_t previous_size = *output_size;
                if (value_size > SIZE_MAX - previous_size) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE,
                                     "Data directive is too large", directive->name);
                }
                *output_size += value_size;
                result = asm_realloc(result, *output_size);
                memcpy(result + previous_size, value, value_size);
                asm_free(value);
            }

            if (directive->type == DIR_ASCIIZ) {
                if (*output_size == SIZE_MAX) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE,
                                     "Data directive is too large", directive->name);
                }
                result = asm_realloc(result, *output_size + 1);
                result[*output_size] = '\0';
                (*output_size)++;
            }
            return result;
        }
        case DIR_BYTE:
            return encode_numeric_data(directive, sizeof(byte), BYTE_MAX, output_size);
        case DIR_HWORD:
            return encode_numeric_data(directive, sizeof(hword), HWORD_MAX, output_size);
        case DIR_WORD:
            return encode_numeric_data(directive, sizeof(word), WORD_MAX, output_size);
        case DIR_DWORD:
            return encode_numeric_data(directive, sizeof(dword), DWORD_MAX, output_size);
        case DIR_SPACE:
            if (directive->argc != 1) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                 ".space expects exactly one argument", directive->name);
            }
            dword value = token_get_int_value(directive->argv[0]);
            if (value > SIZE_MAX) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                                 ".space size exceeds the host size limit", directive->argv[0]);
            }
            *output_size = (size_t)value;
            return asm_calloc(*output_size, sizeof(byte));
        default:
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Directive does not emit raw data",
                             directive->name);
    }
}

static byte *encode_numeric_data(const Directive *directive, size_t element_size,
                                 dword maximum_value, size_t *output_size) {
    if (directive->argc > SIZE_MAX / element_size) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE, "Data directive is too large",
                         directive->name);
    }

    *output_size = directive->argc * element_size;
    byte *result = asm_malloc(*output_size);

    for (size_t index = 0; index < directive->argc; index++) {
        dword value = directive->argv[index]->type == TOK_CHAR_LITERAL
                          ? (byte)token_get_char_value(directive->argv[index])
                          : token_get_int_value(directive->argv[index]);

        if (value > maximum_value) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                             "Value does not fit the directive width", directive->argv[index]);
        }
        for (size_t byte_index = 0; byte_index < element_size; byte_index++) {
            result[index * element_size + byte_index] = (byte)(value >> (byte_index * 8));
        }
    }
    return result;
}

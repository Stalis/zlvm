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
    directive->argc = 0;
    directive->argv = NULL;
    return directive->type != DIR_TOTAL;
}

void directive_add_arg(Directive *directive, Token *argument) {
    if (directive == NULL) {
        ZLASM_TOKEN_CRASH("Directive is null", argument);
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
                byte *value = token_get_raw_data(directive->argv[index], &value_size);
                if (directive->argv[index]->type != TOK_STRING_LITERAL) {
                    value_size = 1;
                }

                size_t previous_size = *output_size;
                *output_size += value_size;
                result = asm_realloc(result, *output_size);
                memcpy(result + previous_size, value, value_size);
                asm_free(value);
            }

            if (directive->type == DIR_ASCIIZ) {
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
                ZLASM_CRASH(".space expects exactly one argument");
            }
            *output_size = (size_t)token_get_int_value(directive->argv[0]);
            return asm_calloc(*output_size, sizeof(byte));
        default:
            ZLASM_CRASH("Directive does not emit raw data");
    }
}

static byte *encode_numeric_data(const Directive *directive, size_t element_size,
                                 dword maximum_value, size_t *output_size) {
    if (directive->argc > SIZE_MAX / element_size) {
        ZLASM_CRASH("Data directive is too large");
    }

    *output_size = directive->argc * element_size;
    byte *result = asm_malloc(*output_size);

    for (size_t index = 0; index < directive->argc; index++) {
        size_t raw_size = 0;
        byte *raw_data = token_get_raw_data(directive->argv[index], &raw_size);
        dword value = 0;
        memcpy(&value, raw_data, raw_size < sizeof value ? raw_size : sizeof value);
        asm_free(raw_data);

        if (value > maximum_value) {
            ZLASM_TOKEN_CRASH("Value does not fit the directive width", directive->argv[index]);
        }
        memcpy(result + index * element_size, &value, element_size);
    }
    return result;
}

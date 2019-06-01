//
// Created by Stanislav on 2019-05-13.
//

#include <memory.h>
#include "../include/Directive.h"

static inline enum DirectiveType get_directive_type(struct Token*);

/*
 * Initialize directive with name as token
 * If name token is valid, sets directive.type
 * @return `true` if directive name is valid, else returns `false`
 */
bool directive_init(struct Directive* d, struct Token* name) {
    assert(d != NULL);
    d->type = get_directive_type(name);
    if (d->type == DIR_TOTAL)
        return false;

    d->argc = 0;
    d->argv = NULL;
    return true;
}

/*
 * Adds argument token to directive statement
 */
void directive_add_arg(struct Directive* d, struct Token* arg) {
    if (d == NULL)
    {
        ZLASM__TOKEN_CRASH("Directive is null", arg);
    }
    assert(d != NULL);
    if (d->argc == 0)
    {
        d->argv = malloc(sizeof(struct Token*));
        if (!d->argv)
            ZLASM__CRASH("Allocation error");
    }
    else
    {
        d->argv = realloc(d->argv, sizeof(struct Token*) * (d->argc + 1));
    }
    d->argv[d->argc++] = arg;
}

/*
 * Free memory, allocated for directive
 */
void directive_free(struct Directive* d) {
    if (NULL != d)
    {
        if (NULL != d->argv)
        {
            free(d->argv);
        }

        free(d);
    }
}

/*
 * Prints directive representation to stdout
 */
void directive_print(struct Directive* d) {
    assert(d != NULL);
}

static inline enum DirectiveType get_directive_type(struct Token* t) {
#define CHECK(str, res) \
    if (strcmp(t->value, str) == 0) \
        return res

    CHECK("section", DIR_SECTION);
    CHECK("global", DIR_GLOBAL); // Declare global symbol
    CHECK("extern", DIR_EXTERN); // Declare external symbol
    CHECK("align", DIR_ALIGN);   // Aligning data below for 2^arg bytes
    CHECK("entry", DIR_ENTRY);   // Declare entry point symbol
    CHECK("locate", DIR_LOCATE); // Static shift code below
    /* Data directives */
    CHECK("ascii", DIR_ASCII);      // Non-null-terminated string
    CHECK("asciiz", DIR_ASCIIZ);    // Null-terminated string
    CHECK("byte", DIR_BYTE);        // sequence of byte(8 bit) values
    CHECK("hword", DIR_HWORD);      // sequence of half word(16 bit) values
    CHECK("word", DIR_WORD);        // sequence of word(32 bit) values
    CHECK("dword", DIR_DWORD);      // sequence of double word(64 bit) values
    CHECK("space", DIR_SPACE);      // empty space for N bytes
    /* Other directives */
    CHECK("macro", DIR_MACRO);
    CHECK("endmacro", DIR_ENDMACRO);
    CHECK("proc", DIR_PROC);        // Begin procedure
    CHECK("endproc", DIR_ENDPROC);  // End procedure

#undef CHECK

    return DIR_TOTAL;
}

byte* directive_get_raw_data(struct Directive* d, size_t* __size) {
#define PARSE_DATA_BY(type)             \
({                                      \
    *__size = d->argc * sizeof(type);   \
    res = malloc(*__size);              \
    ptr = res;                          \
    size_t i = 0;                       \
    for (; i < d->argc; i++)            \
    {                                   \
        size_t size = 0;                \
        dword* data =                   \
                (dword*)token_get_raw_data(d->argv[i], &size);\
        type val = (type)*data;         \
        if (*data != val)               \
            {ZLASM__TOKEN_CRASH("Invalid value size", d->argv[i]);}\
        memcpy(ptr, &val, sizeof(type));\
        ptr++;                          \
    }                                   \
})

    assert(is_data_directive(d->type));
    assert(__size != NULL);
    *__size = 0;

    byte* res = NULL;
    byte* ptr;

    switch (d->type)
    {
        case DIR_ASCII:
        case DIR_ASCIIZ:
            res = malloc(1);
            ptr = res;
            for (size_t i = 0; i < d->argc; i++)
            {
                size_t size = 0;

                byte* data = token_get_raw_data(d->argv[i], &size);
                *__size += size;
                res = realloc(res, *__size);
                memcpy(ptr, data, size);
                ptr += size;
            }
            if (d->type == DIR_ASCIIZ)
            {
                *__size += 1;
                res = realloc(res, *__size);
                res[*__size - 1] = '\0';
            }
            break;
        case DIR_BYTE:
            PARSE_DATA_BY(byte);
            break;
        case DIR_HWORD:
            PARSE_DATA_BY(hword);
            break;
        case DIR_WORD:
            PARSE_DATA_BY(word);
            break;
        case DIR_DWORD:
            PARSE_DATA_BY(dword);

            break;
        case DIR_SPACE:
            *__size = token_get_int_value(d->argv[0]);
            res = malloc(token_get_int_value(d->argv[0]));
        default:
            break;
    }

    return res;
#undef PARSE_DATA_BY
}
#include "Macros.h"

#include <string.h>

#include "Error.h"
#include "Memory.h"

typedef struct Macro {
    Token *name;
    Token **parameters;
    size_t parameter_count;
    TokenList *body;
    TokenList *body_end;
    struct Macro *next;
} Macro;

typedef struct MacroArgument {
    Token *value;
    bool is_set;
} MacroArgument;

static bool is_directive(const Token *token, const char *name) {
    return token != NULL && token->type == TOK_DIRECTIVE && strcmp(token->value, name) == 0;
}

static Token *clone_token(const Token *source, const Token *location) {
    Token *clone = asm_malloc(sizeof *clone);
    *clone = *source;
    clone->value = asm_strdup(source->value);
    if (location != NULL) {
        clone->pos = location->pos;
        clone->line = location->line;
        clone->col = location->col;
        clone->source_size = location->source_size;
    }
    return clone;
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

static Macro *find_macro(Macro *macros, const char *name) {
    for (Macro *macro = macros; macro != NULL; macro = macro->next) {
        if (strcmp(macro->name->value, name) == 0) {
            return macro;
        }
    }
    return NULL;
}

static TokenList *line_end(TokenList *token) {
    while (token != NULL && token->value->type != TOK_NEWLINE) {
        token = token->next;
    }
    return token;
}

static void add_parameter(Macro *macro, Token *parameter) {
    if (parameter->type != TOK_ID) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                         "Macro parameter must be an identifier", parameter);
    }
    for (size_t index = 0; index < macro->parameter_count; index++) {
        if (strcmp(macro->parameters[index]->value, parameter->value) == 0) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT, "Duplicate macro parameter",
                             parameter);
        }
    }
    macro->parameters =
        asm_realloc(macro->parameters, (macro->parameter_count + 1) * sizeof *macro->parameters);
    macro->parameters[macro->parameter_count++] = parameter;
}

static Macro *capture_macro(Macro **macros, TokenList *definition, TokenList **after_definition) {
    TokenList *header_end = line_end(definition);
    TokenList *argument = definition->next;
    if (argument == header_end || argument == NULL || argument->value->type != TOK_ID) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT, ".macro expects a name",
                         definition->value);
    }
    if (find_macro(*macros, argument->value->value) != NULL) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_DUPLICATE_DEFINITION, "Duplicate macro definition",
                         argument->value);
    }

    Macro *macro = asm_calloc(1, sizeof *macro);
    macro->name = argument->value;
    for (argument = argument->next; argument != header_end; argument = argument->next) {
        if (argument->value->type != TOK_COMMA) {
            add_parameter(macro, argument->value);
        }
    }

    TokenList *body = header_end == NULL ? NULL : header_end->next;
    TokenList *current = body;
    while (current != NULL && !is_directive(current->value, "endmacro")) {
        if (is_directive(current->value, "macro")) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                             "Nested macro definitions are not supported", current->value);
        }
        current = current->next;
    }
    if (current == NULL) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT, "Unterminated macro definition",
                         macro->name);
    }
    TokenList *end_line = line_end(current);
    for (argument = current->next; argument != end_line; argument = argument->next) {
        if (argument->value->type != TOK_COMMENT) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                             ".endmacro does not accept arguments", argument->value);
        }
    }

    macro->body = body;
    macro->body_end = current;
    macro->next = *macros;
    *macros = macro;
    *after_definition = end_line == NULL ? NULL : end_line->next;
    return macro;
}

static void capture_macros(Macro **macros, TokenList *tokens) {
    for (TokenList *current = tokens; current != NULL;) {
        if (is_directive(current->value, "macro")) {
            TokenList *after_definition;
            (void)capture_macro(macros, current, &after_definition);
            current = after_definition;
        } else if (is_directive(current->value, "endmacro")) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                             ".endmacro without a matching .macro", current->value);
        } else {
            current = current->next;
        }
    }
}

static TokenList *skip_macro_definition(TokenList *definition) {
    TokenList *current = definition;
    while (current != NULL && !is_directive(current->value, "endmacro")) {
        current = current->next;
    }
    TokenList *end_line = line_end(current);
    return end_line == NULL ? NULL : end_line->next;
}

static size_t parameter_index(const Macro *macro, const char *name) {
    for (size_t index = 0; index < macro->parameter_count; index++) {
        if (strcmp(macro->parameters[index]->value, name) == 0) {
            return index;
        }
    }
    return macro->parameter_count;
}

static Token *named_argument_value(const Token *argument, const char *equals) {
    LexerState lexer;
    lexer_init(&lexer, (char *)(equals + 1));
    Token *value = lexer_readToken(&lexer);
    if (value == NULL || lexer_readToken(&lexer) != NULL) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                         "Macro named argument must contain one value", argument);
    }
    return clone_token(value, argument);
}

static MacroArgument *read_arguments(const Macro *macro, TokenList *first, TokenList *end,
                                     const Token *invocation) {
    MacroArgument *arguments = asm_calloc(macro->parameter_count, sizeof *arguments);
    size_t positional_index = 0;
    bool has_named_argument = false;

    for (TokenList *current = first; current != end; current = current->next) {
        Token *argument = current->value;
        if (argument->type == TOK_COMMA) {
            continue;
        }
        const char *equals = argument->type == TOK_ID ? strchr(argument->value, '=') : NULL;
        if (equals != NULL) {
            size_t name_size = (size_t)(equals - argument->value);
            if (name_size == 0 || equals[1] == '\0') {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                                 "Malformed macro named argument", argument);
            }
            char *name = asm_malloc(name_size + 1);
            memcpy(name, argument->value, name_size);
            name[name_size] = '\0';
            size_t index = parameter_index(macro, name);
            if (index == macro->parameter_count || arguments[index].is_set) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT,
                                 "Unknown or duplicate macro named argument", argument);
            }
            arguments[index].value = named_argument_value(argument, equals);
            arguments[index].is_set = true;
            has_named_argument = true;
        } else {
            if (has_named_argument || positional_index == macro->parameter_count) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT, "Invalid macro argument",
                                 argument);
            }
            arguments[positional_index].value = argument;
            arguments[positional_index].is_set = true;
            positional_index++;
        }
    }

    for (size_t index = 0; index < macro->parameter_count; index++) {
        if (!arguments[index].is_set) {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_INVALID_ARGUMENT, "Missing macro argument",
                             invocation);
        }
    }
    return arguments;
}

static bool is_expanding(const Macro *macro, Macro **stack, size_t stack_count) {
    for (size_t index = 0; index < stack_count; index++) {
        if (stack[index] == macro) {
            return true;
        }
    }
    return false;
}

static void expand_range(Macro *macros, TokenList *first, TokenList *end, Macro **stack,
                         size_t stack_count, TokenList **output_first, TokenList **output_last,
                         const Token *location);

static void expand_macro(Macro *macros, Macro *macro, TokenList *arguments,
                         TokenList *line_end_token, Macro **stack, size_t stack_count,
                         TokenList **output_first, TokenList **output_last,
                         const Token *invocation) {
    if (is_expanding(macro, stack, stack_count)) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_RECURSION, "Recursive macro expansion", invocation);
    }
    if (stack_count == 64) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_RECURSION, "Macro expansion depth exceeds 64",
                         invocation);
    }
    MacroArgument *values = read_arguments(macro, arguments, line_end_token, invocation);
    TokenList *body_first = NULL;
    TokenList *body_last = NULL;
    for (TokenList *current = macro->body; current != macro->body_end; current = current->next) {
        Token *token = current->value;
        size_t index =
            token->type == TOK_ID ? parameter_index(macro, token->value) : macro->parameter_count;
        append_token(
            &body_first, &body_last,
            clone_token(index == macro->parameter_count ? token : values[index].value, invocation));
    }
    stack[stack_count] = macro;
    expand_range(macros, body_first, NULL, stack, stack_count + 1, output_first, output_last,
                 invocation);
}

static void expand_range(Macro *macros, TokenList *first, TokenList *end, Macro **stack,
                         size_t stack_count, TokenList **output_first, TokenList **output_last,
                         const Token *location) {
    bool is_line_start = true;
    for (TokenList *current = first; current != end;) {
        if (is_directive(current->value, "macro")) {
            current = skip_macro_definition(current);
            is_line_start = true;
            continue;
        }
        if (is_line_start && current->value->type == TOK_ID) {
            Token *invocation = current->value;
            const char *name =
                invocation->value[0] == '@' ? invocation->value + 1 : invocation->value;
            Macro *macro = find_macro(macros, name);
            if (macro != NULL || invocation->value[0] == '@') {
                TokenList *end_of_line = line_end(current);
                if (macro == NULL) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_MACRO_UNDEFINED, "Undefined macro",
                                     invocation);
                }
                expand_macro(macros, macro, current->next, end_of_line, stack, stack_count,
                             output_first, output_last, invocation);
                current = end_of_line == NULL ? NULL : end_of_line->next;
                is_line_start = true;
                continue;
            }
        }
        append_token(output_first, output_last, clone_token(current->value, location));
        is_line_start = current->value->type == TOK_NEWLINE ||
                        (is_line_start && current->value->type == TOK_LABEL_INIT);
        current = current->next;
    }
}

TokenList *macros_expand(TokenList *tokens) {
    Macro *macros = NULL;
    capture_macros(&macros, tokens);

    TokenList *expanded = NULL;
    TokenList *last = NULL;
    Macro *stack[64];
    expand_range(macros, tokens, NULL, stack, 0, &expanded, &last, NULL);
    return expanded;
}

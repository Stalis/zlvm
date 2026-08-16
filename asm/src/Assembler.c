// Created by Stanislav on 2019-06-01.
//

#include "Assembler.h"

#include "Instruction.h"
#include "Memory.h"
#include "Registers.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void line_to_upper(char* line) {
    for (char* character = line; *character != '\0'; character++) {
        *character = (char)toupper((unsigned char)*character);
    }
}

static Condition parse_condition(const char* string);
static Register parse_register(const char* string);

void asm_init(AssemblerContext* context) {
    context->entry = NULL;
    context->externals = NULL;
    context->externalsCount = 0;
    context->globals = NULL;
    context->globalsCount = 0;
    context->labels = NULL;
    context->lines = NULL;
}

static const char* const ASM_CONTEXT_DELIMITER = ".";

static const char* set_label_context(const char* context, const char* label);

void asm_processDirectives(AssemblerContext* context, ParserContext* parser) {
    LineList* last = NULL;
    LineStream* stream = lineStream_new(parser->lines);
    parser->lines = NULL;
    parser->lines_count = 0;
    Line* line = lineStream_read(stream);
    const char* procedure_context = NULL;

    while (line != NULL) {
        if (procedure_context != NULL && line->label != NULL) {
            line->label = (char*)set_label_context(procedure_context, line->label);
        }
        if (line->type == L_DIR) {
            if (is_data_directive(line->dir->type)) {
                Directive* dir = line->dir;
                line->type = L_RAW;
                line->raw = asm_calloc(1, sizeof(struct RawData));
                line->raw->data = directive_get_raw_data(dir, &line->raw->size);
                directive_free(dir);
                continue;
            }
            switch (line->dir->type) {
                case DIR_SECTION:
                    // TODO(assembler): implement sections.
                    break;
                case DIR_GLOBAL:
                    asm_addGlobal(context, asm_strdup(line->dir->argv[0]->value));
                    break;
                case DIR_EXTERN:
                    asm_addExternal(context, asm_strdup(line->dir->argv[0]->value));
                    break;
                case DIR_ALIGN:
                    // TODO(assembler): implement data alignment.
                    break;
                case DIR_ENTRY:
                    context->entry = asm_strdup(line->dir->argv[0]->value);
                    break;
                case DIR_LOCATE:
                    // TODO(assembler): implement static location changes.
                    break;
                case DIR_PROC:
                    if (stream->first == NULL || stream->first->value == NULL) {
                        ZLASM_CRASH(".proc must be followed by a procedure body");
                    }
                    stream->first->value->label = "";
                    procedure_context = line->dir->argv[0]->value;
                    break;
                case DIR_ENDPROC:
                    procedure_context = NULL;
                    break;
                case DIR_MACRO:
                    // TODO(assembler): implement macros.
                    break;
                case DIR_ENDMACRO:
                    break;
                default:
                    ZLASM_CRASH("Invalid directive");
            }

            directive_free(line->dir);
            asm_free(line);
            line = NULL;
        }

        if (line != NULL) {
            if (context->lines == NULL) {
                context->lines = asm_calloc(1, sizeof(LineList));
            }
            if (last == NULL) {
                last = context->lines;
            } else {
                last->next = asm_calloc(1, sizeof(LineList));
                last = last->next;
            }
            last->value = line;
        }

        line = lineStream_read(stream);
    }
    asm_free(stream);
}

void asm_addGlobal(AssemblerContext* context, const char* symbol) {
    if (context->globals == NULL) {
        context->globals = asm_malloc(sizeof(const char*));
    } else {
        context->globals =
            asm_realloc(context->globals, sizeof(const char*) * (context->globalsCount + 1));
    }
    context->globals[context->globalsCount++] = symbol;
}

void asm_addExternal(AssemblerContext* context, const char* symbol) {
    if (context->externals == NULL) {
        context->externals = asm_malloc(sizeof(const char*));
    } else {
        context->externals =
            asm_realloc(context->externals, sizeof(const char*) * (context->externalsCount + 1));
    }
    context->externals[context->externalsCount++] = symbol;
}

void asm_processLabels(AssemblerContext* context) {
    LineList* last = context->lines;
    context->labels = asm_calloc(1, sizeof(LabelTable));
    size_t address = 0;

    while (last != NULL) {
        if (last->value->label != NULL) {
            labelTable_setOrCreate(context->labels, asm_strdup(last->value->label), address);
        }
        if (last->value->type == L_STMT) {
            address += sizeof(Instruction);
        } else if (last->value->type == L_RAW) {
            address += last->value->raw->size;
        }
        last = last->next;
    }
}

byte* asm_translate(AssemblerContext* context, size_t* output_size) {
    const size_t growth_size = 1024;
    size_t capacity = growth_size;
    size_t offset = 0;
    byte* result = asm_calloc(capacity, sizeof *result);

    for (LineList* current = context->lines; current != NULL; current = current->next) {
        const byte* data = NULL;
        size_t data_size = 0;
        Instruction instruction = {0};

        if (current->value->type == L_STMT) {
            Statement* statement = current->value->stmt;
            line_to_upper(statement->opcode->value);
            instruction.opcode_ = string_to_opcode(statement->opcode->value);
            if (instruction.opcode_ == OPCODE_TOTAL) {
                ZLASM_TOKEN_CRASH("Unknown opcode", statement->opcode);
            }

            instruction.condition_ =
                statement->cond == NULL ? C_UNCONDITIONAL : parse_condition(statement->cond->value);
            instruction.register1 =
                statement->reg1 == NULL ? R_ZERO : parse_register(statement->reg1->value);
            instruction.register2 =
                statement->reg2 == NULL ? R_ZERO : parse_register(statement->reg2->value);

            if (statement->imm != NULL) {
                if (statement->imm->type == TOK_LABEL_USE) {
                    LabelInfo* label = labelInfo_getIfExist(context->labels, statement->imm->value);
                    if (label == NULL) {
                        ZLASM_TOKEN_CRASH("Unknown label", statement->imm);
                    }
                    instruction.immediate = (word)label->address;
                } else if (statement->imm->type == TOK_CHAR_LITERAL) {
                    instruction.immediate = token_get_char_value(statement->imm->value);
                } else {
                    instruction.immediate = (word)token_get_int_value(statement->imm);
                }
            }

            data = (const byte*)&instruction;
            data_size = sizeof instruction;
        } else if (current->value->type == L_RAW) {
            data = current->value->raw->data;
            data_size = current->value->raw->size;
        } else {
            ZLASM_CRASH("Assembler error: invalid line type");
        }

        if (data_size > SIZE_MAX - offset) {
            ZLASM_CRASH("Assembler output is too large");
        }
        size_t required_size = offset + data_size;
        if (required_size > capacity) {
            while (required_size > capacity) {
                if (capacity > SIZE_MAX - growth_size) {
                    ZLASM_CRASH("Assembler output is too large");
                }
                capacity += growth_size;
            }
            result = asm_realloc(result, capacity);
        }

        memcpy(result + offset, data, data_size);
        offset = required_size;
    }

    *output_size = offset;
    return result;
}

static const char* set_label_context(const char* context, const char* label) {
    if (strlen(label) == 0) {
        return asm_strdup(context);
    }
    size_t size = strlen(context) + strlen(ASM_CONTEXT_DELIMITER) + strlen(label) + 1;
    char* result = asm_malloc(size);
    snprintf(result, size, "%s%s%s", context, ASM_CONTEXT_DELIMITER, label);
    return result;
}

static Condition parse_condition(const char* string) {

#define CHECK(str, code)                                                                           \
    if (strcmp(string, str) == 0) {                                                                \
        return code;                                                                               \
    }

    CHECK("un", C_UNCONDITIONAL);

    CHECK("zs", C_ZERO_SET);
    CHECK("eq", C_ZERO_SET);
    CHECK("zc", C_ZERO_CLEAR);
    CHECK("ne", C_ZERO_CLEAR);

    CHECK("cs", C_CARRY_SET);
    CHECK("hs", C_CARRY_SET); // unsigned higher or same
    CHECK("cc", C_CARRY_CLEAR);
    CHECK("lo", C_CARRY_CLEAR); // unsigned lower

    CHECK("ns", C_NEGATIVE_SET);
    CHECK("mi", C_NEGATIVE_SET);
    CHECK("nc", C_NEGATIVE_CLEAR);
    CHECK("pl", C_NEGATIVE_CLEAR);

    CHECK("vs", C_OVERFLOW_SET);
    CHECK("vc", C_OVERFLOW_CLEAR);

    CHECK("ss", C_SIGNED_SET);
    CHECK("sc", C_SIGNED_CLEAR);

    CHECK("gt", C_GREATER);
    CHECK("ge", C_GREATER_OR_EQUALS);
    CHECK("lt", C_LESS);
    CHECK("le", C_LESS_OR_EQUALS);

    ZLASM_CRASH("Unknown condition");
#undef CHECK
}

static Register parse_register(const char* string) {
    if (string == NULL) {
        return R_ZERO;
    }

    if (string[0] == 'r') {
        char* end = NULL;
        unsigned long index = strtoul(string + 1, &end, 10);
        if (*end != '\0' || index >= R_TOTAL) {
            ZLASM_CRASH("Invalid numeric register");
        }
        return (Register)index;
    }

#define CHECK(str, reg)                                                                            \
    if (strcmp(string, str) == 0) {                                                                \
        return reg;                                                                                \
    }

    CHECK("zero", R_ZERO);
    CHECK("at", R_AT);

    CHECK("v0", R_V0);
    CHECK("v1", R_V1);
    CHECK("v2", R_V2);
    CHECK("v3", R_V3);

    CHECK("a0", R_A0);
    CHECK("a1", R_A1);
    CHECK("a2", R_A2);
    CHECK("a3", R_A3);

    CHECK("t0", R_T0);
    CHECK("t1", R_T1);
    CHECK("t2", R_T2);
    CHECK("t3", R_T3);
    CHECK("t4", R_T4);
    CHECK("t5", R_T5);
    CHECK("t6", R_T6);
    CHECK("t7", R_T7);

    CHECK("s0", R_S0);
    CHECK("s1", R_S1);
    CHECK("s2", R_S2);
    CHECK("s3", R_S3);
    CHECK("s4", R_S4);
    CHECK("s5", R_S5);
    CHECK("s6", R_S6);
    CHECK("s7", R_S7);

    CHECK("k0", R_K0);

    CHECK("sc", R_SC);
    CHECK("lp", R_LP);
    CHECK("sp", R_SP);
    CHECK("bp", R_BP);
    CHECK("pc", R_PC);

#undef CHECK
    ZLASM_CRASH("Unknown register");
}

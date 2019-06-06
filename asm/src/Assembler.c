// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-06-01.
//

#include <ctype.h>

#include "Assembler.h"
#include "Memory.h"
#include "../../emulator/include/Instruction.h"
#include "../../emulator/include/Registers.h"
#include "../../emulator/include/zlvm.h"

inline static void line_to_lower(char* line) {
    char* p = line;
    for (; *p; ++p) *p = (char) tolower(*p);
}

inline static void line_to_upper(char* line) {
    char* p = line;
    for (; *p; ++p) *p = (char) toupper(*p);
}

inline static Condition parseCondition(const char*);
inline static Register parseRegister(const char*);

void asm_init(AssemblerContext* ctx) {
    ctx->entry = NULL;
    ctx->externals = NULL;
    ctx->externalsCount = 0;
    ctx->globals = NULL;
    ctx->globalsCount = 0;
    ctx->labels = NULL;
    ctx->lines = NULL;
}

static const char* CONTEXT_DELIMITER = ".";

static const char* set_label_context(const char* ctx, const char* label);

void asm_processDirectives(AssemblerContext* ctx, ParserContext* parser) {
    LineList* last = NULL;
    LineStream* stream = lineStream_new(parser->lines);
    Line* line = lineStream_read(stream);
    char* proc_context = NULL;

#define REMOVE_LINE                 \
({                                  \
    void* __buf__ = line;           \
    line = NULL;                    \
    free(__buf__);                  \
})
// REMOVE_LINE

    while (line != NULL)
    {
        if (proc_context != NULL && line->label != NULL)
        {
            const char* buf = line->label;
            line->label = (char*) set_label_context(proc_context, buf);
        }
        if (line->type == L_DIR)
        {
            if (is_data_directive(line->dir->type))
            {
                Directive* dir = line->dir;
                line->type = L_RAW;
                line->raw = calloc(1, sizeof(struct RawData));
                line->raw->data = directive_get_raw_data(dir, &line->raw->size);
                continue;
            }
            switch (line->dir->type)
            {
                case DIR_SECTION:
                    // TODO: Add sections
                    break;
                case DIR_GLOBAL:
                    asm_addGlobal(ctx, strdup(line->dir->argv[0]->value));
                    break;
                case DIR_EXTERN:
                    asm_addExternal(ctx, strdup(line->dir->argv[0]->value));
                    break;
                case DIR_ALIGN:
                    // TODO: Add aligning data
                    break;
                case DIR_ENTRY:
                    ctx->entry = strdup(line->dir->argv[0]->value);
                    break;
                case DIR_LOCATE:
                    // TODO: Add static location shift
                    break;
                case DIR_PROC:
                    stream->first->value->label = "";
                    proc_context = line->dir->argv[0]->value;
                    break;
                case DIR_ENDPROC:
                    proc_context = NULL;
                    break;
                case DIR_MACRO:
                    // TODO: Add macroses
                    break;
                case DIR_ENDMACRO:
                    break;
                default:
                    ZLASM__CRASH("Invalid directive");
            }

            void* __buf__ = line;
            line = NULL;
            free(__buf__);
        }

        if (line != NULL)
        {

            if (ctx->lines == NULL)
            {
                ctx->lines = calloc(1, sizeof(LineList));
            }
            if (last == NULL)
            {
                last = ctx->lines;
            }
            else
            {
                last->next = calloc(1, sizeof(LineList));
                last = last->next;
            }
            last->value = line;
        }

        line = lineStream_read(stream);
    }
#undef REMOVE_LINE
}

void asm_addGlobal(AssemblerContext* ctx, const char* sym) {
    if (ctx->globals == NULL)
    {
        ctx->globals = malloc(sizeof(const char*));
    }
    else
    {
        ctx->globals = realloc(ctx->globals, sizeof(const char*) * (ctx->globalsCount + 1));
    }
    ctx->globals[ctx->globalsCount++] = sym;
}

void asm_addExternal(AssemblerContext* ctx, const char* sym) {
    if (ctx->externals == NULL)
    {
        ctx->externals = malloc(sizeof(const char*));
    }
    else
    {
        ctx->externals = realloc(ctx->externals, sizeof(const char*) * (ctx->externalsCount + 1));
    }
    ctx->externals[ctx->externalsCount++] = sym;
}

void asm_processLabels(AssemblerContext* ctx) {
    LineList* last = ctx->lines;
    ctx->labels = calloc(1, sizeof(LabelTable));
    size_t addr = 0;

    while (last != NULL)
    {
        if (last->value->label != NULL)
        {
            labelTable_setOrCreate(ctx->labels, strdup(last->value->label), addr);
#ifdef DEBUG
            printf("%s: 0x%lX\n", last->value->label, addr);
#endif
        }
        if (last->value->type == L_STMT)
        {
            addr += sizeof(Instruction);
        }
        else if (last->value->type == L_RAW)
        {
            addr += last->value->raw->size;
        }
        last = last->next;
    }
}

byte* asm_translate(AssemblerContext* ctx, size_t* __size) {
    LineList* cur = ctx->lines;
    const size_t bufferStep = 1024;
    size_t bufferSize = bufferStep;
    byte* result = calloc(bufferSize, sizeof(byte));
    byte* ptr = result;
    size_t length = 0;
    byte* buf;

    while (cur != NULL)
    {
        if (cur->value->type == L_STMT)
        {
            Statement* stmt = cur->value->stmt;
            line_to_upper(stmt->opcode->value);
            Instruction instr;

            instr.opcode_ = string_to_opcode(stmt->opcode->value);

            if (stmt->cond != NULL)
                instr.condition_ = parseCondition(stmt->cond->value);
            else
                instr.condition_ = C_UNCONDITIONAL;

            if (stmt->reg1 != NULL)
                instr.register1 = parseRegister(stmt->reg1->value);
            else
                instr.register1 = R_ZERO;

            if (stmt->reg2 != NULL)
                instr.register2 = parseRegister(stmt->reg2->value);
            else
                instr.register2 = R_ZERO;

            if (stmt->imm != NULL)
            {
                if (stmt->imm->type == TOK_LABEL_USE)
                {
                    LabelInfo* l = labelInfo_getIfExist(ctx->labels, stmt->imm->value);
                    if (l != NULL)
                    {
                        instr.immediate = l->address;
                    }
                    else
                    {
                        ZLASM__TOKEN_CRASH("Unknown label", stmt->imm);
                    }
                }
                else
                {
                    switch (stmt->imm->type)
                    {
                        case TOK_CHAR_LITERAL:
                            instr.immediate = token_get_char_value(stmt->imm->value);
                            break;
                        case TOK_INT_HEX:
                        case TOK_INT_DEC:
                        case TOK_INT_OCT:
                        case TOK_INT_BIN:
                            instr.immediate = token_get_int_value(stmt->imm);
                            break;
                        default:
                            ZLASM__TOKEN_CRASH("Invalid immediate value", stmt->imm);
                    }
                }
            }
            else
                instr.immediate = 0;

            length = sizeof(Instruction);
            buf = (byte*) &instr;
            instruction_print(&instr);
        }
        else if (cur->value->type == L_RAW)
        {
            length = cur->value->raw->size;
            buf = cur->value->raw->data;
        }
        else
        {
            ZLASM__CRASH("Assembler error: invalid line type");
        }

        while ((size_t) labs((ptr + length) - result) >= bufferSize)
        {
            bufferSize += bufferStep;
            result = realloc(result, bufferSize);
        }

        memcpy(ptr, buf, length);
        ptr += length;
        length = 0;
        cur = cur->next;
    }

    *__size = (size_t) labs(ptr - result);
    return result;
}

static const char* set_label_context(const char* ctx, const char* label) {
    if (strlen(label) == 0)
    {
        return strdup(ctx);
    }
    char* buf = calloc(256, sizeof(char));
    sprintf(buf, "%s%s%s", ctx, CONTEXT_DELIMITER, label);
    buf = realloc(buf, strlen(buf));

    return buf;
}

inline static Condition parseCondition(const char* s) {

#define CHECK(str, code) \
    if (strcmp(s, str) == 0) { \
        return code; \
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

    ZLASM__CRASH("Unknown condition");
#undef CHECK
}

inline static Register parseRegister(const char* s) {
    if (NULL == s)
    {
        return R_ZERO;
    }

    if (s[0] == 'r')
    {
        char** end = NULL;
        return (byte) strtoul(s + 1, end, 10);
    }

#define CHECK(str, reg) \
    if (strcmp(s, str) == 0) { \
        return reg; \
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
    return R_ZERO;
}

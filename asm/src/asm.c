//
// Created by Stanislav on 2019-04-29.
//
#include <string.h>
#include <ctype.h>
#include "../../emulator/include/Types.h"
#include "../../emulator/include/Registers.h"
#include "../include/asm.h"

inline static void line_to_lower(char* line) {
    char* p = line;
    for (; *p; ++p) *p = (char) tolower(*p);
}

inline static void line_to_upper(char* line) {
    char* p = line;
    for (; *p; ++p) *p = (char) toupper(*p);
}

struct Instruction parseLine(char* line) {
    bool error = false;

    enum Opcode op = NOP;
    enum Condition cond = C_UNCONDITIONAL;
    byte reg1 = 0;
    byte reg2 = 0;
    word imm = 0;

    byte* preg1 = NULL;

    char* part = strtok(line, partsDelimiters);
    if (!parseOpcode(part, &op))
    {
        exit(EXIT_CODE_INVALID_OPERATION);
    }

    part = strtok(NULL, partsDelimiters);
    while (NULL != part)
    {
        if (parseCondition(part, &cond))
        {
            goto __next_part;
        }

        byte reg = 0;
        if (parseRegister(part, &reg))
        {
            if (NULL == preg1)
            {
                reg1 = reg;
                preg1 = &reg1;
            }
            else
            {
                reg2 = reg;
            }
            goto __next_part;
        }

        if (parseImmediate(part, &imm))
        {
            goto __next_part;
        }

    __next_part:
        part = strtok(NULL, partsDelimiters);
    }

    struct Instruction res = {
            .opcode_ = op,
            .condition_ = cond,
            .register1 = reg1,
            .register2 = reg2,
            .immediate = imm,
    };

    return res;
}

char** getLines(char* source, size_t* lines_count) {
    const size_t step_size = 10;
    size_t cur_size = step_size;
    char** lines = (char**) malloc(sizeof(char*) * cur_size);
    size_t i = 0;

    char* line = strtok(source, lineDelimiters);
    while (NULL != line)
    {
        if (strlen(line) == 0)
            continue;

        lines[i] = line;
        i++;
        if (i >= cur_size)
        {
            cur_size += step_size;
            lines = (char**) realloc(lines, sizeof(char*) * cur_size);
        }
        line = strtok(NULL, lineDelimiters);
    }

    *lines_count = i;
    return lines;
}

struct Instruction* parseFile(char* source, size_t* length) {
    size_t lines_count = 0;
    char** lines = getLines(source, &lines_count);
    struct Instruction* instructions = (struct Instruction*) malloc(sizeof(struct Instruction) * lines_count);

    for (size_t i = 0; i < lines_count; i++)
    {
        instructions[i] = parseLine(lines[i]);
    }

    free(lines);
    *length = lines_count;
    return instructions;
}

byte* assembly(char* source, size_t* size) {
    size_t count = 0;
    byte* buf = (byte*) parseFile(source, &count);
    *size = count * sizeof(struct Instruction);
    return buf;
}

bool parseOpcode(char* mnemonic, enum Opcode* res) {
    if (mnemonic == NULL)
    {
        return false;
    }
    else
        line_to_upper(mnemonic);

    *res = string_to_opcode(mnemonic);
    if (*res == OPCODE_TOTAL)
    {
        return false;
    }
    return true;
}

bool parseCondition(char* mnemonic, enum Condition* res) {
    if (mnemonic == NULL)
    {
        *res = C_UNCONDITIONAL;
        return true;
    }
    else
        line_to_lower(mnemonic);

    size_t last = strlen(mnemonic) - 1;
    if (mnemonic[last] == -1)
    {
        mnemonic[last] = '\0';
    }

#define CHECK(str, code) \
    if (strcmp(mnemonic, str) == 0) { \
        *res = code; \
        return true; \
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

    return false;
#undef CHECK
}

bool parseRegister(char* source, byte* res) {
    if (NULL == source)
    {
        return false;
    }

    if (source[0] == 'r')
    {
        *res = parseByte(source + 1);
        return true;
    }

#define CHECK(str, reg) \
    if (strcmp(str, source) == 0) { \
        *res = reg; \
        return true; \
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

    return false;
}

bool parseImmediate(char* source, word* res) {
    if (NULL == source)
    {
        return false;
    }

    char* base_header = malloc(sizeof(char) * 3);
    strncpy(base_header, source, sizeof(char) * 2);

    line_to_upper(base_header);

    if (strcmp(base_header, HexHeader) == 0)
    {
        *res = parseWord(source + 2, 16);
    }
    else if (strcmp(base_header, OctHeader) == 0)
    {
        *res = parseWord(source + 2, 8);
    }
    else if (strcmp(base_header, BinHeader) == 0)
    {
        *res = parseWord(source + 2, 2);
    }
    else
    {
        *res = parseWord(source, 10);
    }

    free(base_header);
    return true;
}

byte parseByte(char* source) {
    if (source == NULL)
        return 0;

    char** end = NULL;
    return (byte) strtoul(source, end, 10);
}

word parseWord(char* source, byte base) {
    if (source == NULL)
        return 0;

    char** end = NULL;
    return (word) strtoul(source, end, base);
}
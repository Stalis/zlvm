//
// Created by Stanislav on 2019-04-29.
//
#include <string.h>
#include <ctype.h>
#include "../common/Types.h"
#include "asm.h"

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
    if (!parseOpcode(part, &op)) {
        exit(EXIT_CODE_INVALID_OPERATION);
    }

    part = strtok(NULL, partsDelimiters);
    while (NULL != part) {
        if (parseCondition(part, &cond)) {
            goto __next_part;
        }

        byte reg = 0;
        if (parseRegister(part, &reg)) {
            if (NULL == preg1) {
                reg1 = reg;
                preg1 = &reg1;
            } else {
                reg2 = reg;
            }
            goto __next_part;
        }

        if (parseImmediate(part, &imm)) {
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
    while (NULL != line) {
        if (strlen(line) == 0)
            continue;

        lines[i] = line;
        i++;
        if (i >= cur_size) {
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

    for (size_t i = 0; i < lines_count; i++) {
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
    if (mnemonic == NULL) {
        return false;
    }
    else
        line_to_upper(mnemonic);

    *res = string_to_opcode(mnemonic);
    if (*res == OPCODE_TOTAL) {
        return false;
    }
    return true;
}

bool parseCondition(char* mnemonic, enum Condition* res) {
    if (mnemonic == NULL) {
        *res = C_UNCONDITIONAL;
        return true;
    }
    else
        line_to_lower(mnemonic);

    size_t last = strlen(mnemonic) - 1;
    if (mnemonic[last] == -1) {
        mnemonic[last] = '\0';
    }

#define CHECK(str, code) \
    if (strcmp(mnemonic, str) == 0) { \
        *res = code; \
        return true; \
    }

    CHECK("un", C_UNCONDITIONAL);
    CHECK("eq", C_EQUALS);
    CHECK("ne", C_NOT_EQUALS);
    CHECK("gt", C_GREATER);
    CHECK("ge", C_GREATER_OR_EQUALS);
    CHECK("lt", C_LESS);
    CHECK("le", C_LESS_OR_EQUALS);

    CHECK("n", C_NEGATIVE);
    CHECK("z", C_ZERO);
    CHECK("v", C_OVERFLOW);
    CHECK("c", C_CARRY);
    CHECK("s", C_SIGNED);

    return false;
#undef CHECK
}

bool parseRegister(char* source, byte* res) {
    if (NULL == source) {
        return false;
    }

    if (source[0] == 'r') {
        *res = parseByte(source + 1);
        return true;
    }

    return false;
}

bool parseImmediate(char* source, word* res) {
    if (NULL == source) {
        return false;
    }

    char* base_header = malloc(sizeof(char) * 3);
    strncpy(base_header, source, sizeof(char) * 2);

    line_to_upper(base_header);

    if (strcmp(base_header, HexHeader) == 0) {
        *res = parseWord(source + 2, 16);
    } else if (strcmp(base_header, OctHeader) == 0) {
        *res = parseWord(source + 2, 8);
    } else if (strcmp(base_header, BinHeader) == 0) {
        *res = parseWord(source + 2, 2);
    } else {
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
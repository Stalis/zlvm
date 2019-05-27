//
// Created by Stanislav on 2019-04-29.
//

#ifndef ZLVM_C_ASM_H
#define ZLVM_C_ASM_H

#include "../../emulator/include/Instruction.h"

static const char lineDelimiters[] = "\n\r";
static const char partsDelimiters[] = " \t";
static const char commentDelimiters[] = ";";

static const char HexHeader[] = "0X";
static const char OctHeader[] = "0O";
static const char BinHeader[] = "0B";

static const sword EXIT_CODE_INVALID_OPERATION = -10;
static const sword EXIT_CODE_INVALID_CONDITION = -11;
static const sword EXIT_CODE_INVALID_REGISTER = -12;
static const sword EXIT_CODE_INVALID_IMMEDIATE = -13;

byte* assembly(char* source, size_t* size);
struct Instruction* parseFile(char* source, size_t* length);
struct Instruction parseLine(char*);

bool parseOpcode(char*, enum Opcode*);
bool parseCondition(char*, enum Condition*);
bool parseRegister(char*, byte*);
bool parseImmediate(char*, word*);

byte parseByte(char*);
word parseWord(char*, byte base);

#endif //ZLVM_C_ASM_H

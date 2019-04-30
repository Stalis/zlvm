//
// Created by Stanislav on 2019-04-29.
//

#ifndef ZLVM_C_ASM_H
#define ZLVM_C_ASM_H

#include "../common/Instruction.h"

static const char lineDelimiters[] = "\n\r";
static const char partsDelimiters[] = " \t";

byte* assembly(char* source, size_t* size);
struct Instruction* parseFile(char* source, size_t* length);
struct Instruction parseLine(char*);
enum Opcode parseOpcode(char*);
enum Condition parseCondition(char*);
byte parseByte(char*);
word parseWord(char*);

#endif //ZLVM_C_ASM_H

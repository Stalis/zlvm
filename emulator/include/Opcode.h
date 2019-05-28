//
// Created by Stanislav on 2019-05-01.
//

#ifndef ZLVM_C_OPCODE_H
#define ZLVM_C_OPCODE_H

#include <string.h>
#include "Types.h"

#define FOREACH_OPCODE(OPCODE) \
    OPCODE(NOP)\
    OPCODE(POP)\
    OPCODE(POPR)\
    OPCODE(PUSHR)\
    OPCODE(PUSHI)\
    OPCODE(DUP)\
    OPCODE(MOVR)\
    OPCODE(MOVI)\
    OPCODE(ADDR)\
    OPCODE(SUBR)\
    OPCODE(MULR)\
    OPCODE(DIVR)\
    OPCODE(MODR)\
    OPCODE(ADDI)\
    OPCODE(SUBI)\
    OPCODE(MULI)\
    OPCODE(DIVI)\
    OPCODE(MODI)\
    OPCODE(INT)\
    OPCODE(SYSCALL)\
    OPCODE(JMP)\
    OPCODE(NOT)\
    OPCODE(ANDR)\
    OPCODE(ORR)\
    OPCODE(XORR)\
    OPCODE(NANDR)\
    OPCODE(NORR)\
    OPCODE(ANDI)\
    OPCODE(ORI)\
    OPCODE(XORI)\
    OPCODE(NANDI)\
    OPCODE(NORI)\
    OPCODE(INC)\
    OPCODE(DEC)\
    OPCODE(LOADB)\
    OPCODE(STOREB)\
    OPCODE(LOADH)\
    OPCODE(STOREH)\
    OPCODE(LOADW)\
    OPCODE(STOREW)\
    OPCODE(JMPAL)\
    OPCODE(RET)\
    OPCODE(CMPR)\
    OPCODE(CMPI)\
    OPCODE(CMPSR)\
    OPCODE(CMPSI)\
    OPCODE(ADDSR)\
    OPCODE(SUBSR)\
    OPCODE(MULSR)\
    OPCODE(DIVSR)\
    OPCODE(MODSR)\
    OPCODE(ADDSI)\
    OPCODE(SUBSI)\
    OPCODE(MULSI)\
    OPCODE(DIVSI)\
    OPCODE(MODSI)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum Opcode {
    FOREACH_OPCODE(GENERATE_ENUM)
    OPCODE_TOTAL,
};

static const char* opcode_strings[] = {
        FOREACH_OPCODE(GENERATE_STRING)
};

#undef FOREACH_OPCODE
#undef GENERATE_ENUM
#undef GENERATE_STRING

/**
 * Returns opcode mnemonic
 * @param op opcode
 * @return mnemonic's null-terminated string
 */
const char* opcode_to_string(enum Opcode op);

/**
 * Parse opcode mnemonic
 * @param string mnemonic null-terminated string
 * @return opcode
 */
enum Opcode string_to_opcode(const char* string);

#endif //ZLVM_C_OPCODE_H

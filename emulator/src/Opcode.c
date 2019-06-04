// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-27.
//

#include "../include/Opcode.h"

/**
 * Returns opcode mnemonic
 * @param op opcode
 * @return mnemonic's null-terminated string
 */
const char* opcode_to_string(Opcode op) {
    if (op >= OPCODE_TOTAL)
    {
        return NULL;
    }
    else
    {
        return opcode_strings[op];
    }
}

/**
 * Parse opcode mnemonic
 * @param string mnemonic null-terminated string
 * @return opcode
 */
Opcode string_to_opcode(const char* string) {
    for (size_t i = 0; i < OPCODE_TOTAL; i++)
    {
        if (strcmp(string, opcode_strings[i]) == 0)
        {
            return (enum Opcode) i;
        }
    }

    return OPCODE_TOTAL;
}
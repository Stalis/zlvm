// Created by Stanislav on 2019-05-27.
//

#include "Opcode.h"

#define GENERATE_STRING(STRING) #STRING,

static const char *const opcode_strings[] = {FOREACH_OPCODE(GENERATE_STRING)};

#undef FOREACH_OPCODE
#undef GENERATE_STRING

/**
 * Returns opcode mnemonic
 * @param op opcode
 * @return mnemonic's null-terminated string
 */
const char *opcode_to_string(Opcode op) {
    if (op >= OPCODE_TOTAL) {
        return NULL;
    } else {
        return opcode_strings[op];
    }
}

/**
 * Parse opcode mnemonic
 * @param string mnemonic null-terminated string
 * @return opcode
 */
Opcode string_to_opcode(const char *string) {
    for (size_t index = 0; index < OPCODE_TOTAL; index++) {
        if (strcmp(string, opcode_strings[index]) == 0) {
            return (Opcode)index;
        }
    }

    return OPCODE_TOTAL;
}

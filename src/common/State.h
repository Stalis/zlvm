//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_ERROR_H
#define ZLVM_C_ERROR_H

#include <stdbool.h>

enum State {
    S_NORMAL = 0,

    S_ERR_BEGIN,
    S_ERR_OUT_OF_MEMORY,
    S_ERR_STACK_OVERFLOW,
    S_ERR_STACK_UNDERFLOW,
    S_ERR_INVALID_CONDITION,
    S_ERR_INVALID_OPCODE,
    S_ERR_END,

    S_HALTED,
    S_TOTAL,
};

static inline bool is_error(enum State state) {
    return (state >= S_ERR_BEGIN) && (state <= S_ERR_END);
}

#endif //ZLVM_C_ERROR_H

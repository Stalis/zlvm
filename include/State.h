//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_STATE_H
#define ZLVM_C_STATE_H

#include <stdbool.h>

typedef enum State {
    S_HALTED = 0,
    S_NORMAL,

    S_ERR_BEGIN,
    S_ERR_OUT_OF_MEMORY,
    S_ERR_STACK_OVERFLOW,
    S_ERR_STACK_UNDERFLOW,
    S_ERR_INVALID_CONDITION,
    S_ERR_INVALID_OPCODE,
    S_ERR_END,

    S_TOTAL,
} State;

/**
 * Check state for errors
 * @param state
 * @return `true` is state is error state
 */
static inline bool is_error(State state) {
    return (state >= S_ERR_BEGIN) && (state <= S_ERR_END);
}

#endif //ZLVM_C_STATE_H

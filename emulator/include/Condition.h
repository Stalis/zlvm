//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CONDITION_H
#define ZLVM_C_CONDITION_H

typedef enum Condition {
    C_UNCONDITIONAL = 0,
    C_ZERO_SET,
    C_ZERO_CLEAR,
    C_NEGATIVE_SET,
    C_NEGATIVE_CLEAR,
    C_OVERFLOW_SET,
    C_OVERFLOW_CLEAR,
    C_CARRY_SET,
    C_CARRY_CLEAR,
    C_SIGNED_SET,
    C_SIGNED_CLEAR,
    C_UNSIGNED_HIGHER,
    C_UNSIGNED_LOWER_OR_SAME,
    C_LESS,
    C_LESS_OR_EQUALS,
    C_GREATER,
    C_GREATER_OR_EQUALS,
    C_TOTAL,
} Condition;

static const char* condition_to_string(Condition c) {
    switch (c)
    {
        case C_UNCONDITIONAL:
            return "un";
        case C_ZERO_SET:
            return "zs";
        case C_ZERO_CLEAR:
            return "zc";
        case C_NEGATIVE_SET:
            return "ns";
        case C_NEGATIVE_CLEAR:
            return "nc";
        case C_OVERFLOW_SET:
            return "vs";
        case C_OVERFLOW_CLEAR:
            return "vc";
        case C_CARRY_SET:
            return "cs";
        case C_CARRY_CLEAR:
            return "cc";
        case C_SIGNED_SET:
            return "ss";
        case C_SIGNED_CLEAR:
            return "sc";
        case C_UNSIGNED_HIGHER:
            return "uh";
        case C_UNSIGNED_LOWER_OR_SAME:
            return "ul";
        case C_LESS:
            return "lt";
        case C_LESS_OR_EQUALS:
            return "le";
        case C_GREATER:
            return "gt";
        case C_GREATER_OR_EQUALS:
            return "ge";
        case C_TOTAL:
            return "ERROR";
    }
}

#endif //ZLVM_C_CONDITION_H

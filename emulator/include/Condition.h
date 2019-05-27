//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CONDITION_H
#define ZLVM_C_CONDITION_H

enum Condition {
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
};

#endif //ZLVM_C_CONDITION_H
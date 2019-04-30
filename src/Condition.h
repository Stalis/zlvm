//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CONDITION_H
#define ZLVM_C_CONDITION_H

enum Condition {
    C_UNCONDITIONAL = 0,
    C_EQUALS,
    C_NOT_EQUALS,
    C_LESS,
    C_LESS_OR_EQUALS,
    C_GREATER,
    C_GREATER_OR_EQUALS,
    C_NEGATIVE,
    C_ZERO,
    C_OVERFLOW,
    C_CARRY,
    C_SIGNED,
    C_TOTAL,
};

#endif //ZLVM_C_CONDITION_H

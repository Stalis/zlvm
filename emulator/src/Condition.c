#include "Condition.h"

const char *condition_to_string(Condition condition) {
    switch (condition) {
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
        default:
            return "ERROR";
    }
}

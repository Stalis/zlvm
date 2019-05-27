//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CPSR_H
#define ZLVM_C_CPSR_H

#include "../include/Types.h"
#include "../include/State.h"
#include "Value.h"

union CPSR {
    union Value value_;
    struct {
        bool N : 1; // negative
        bool Z : 1; // zero
        bool C : 1; // carry
        bool V : 1; // overflow
        bool S : 1; // last operation is signed
        byte _reserved : 3;
        enum State ST : 8;
        dword _reserverd1 : 48;
    };
};

#endif //ZLVM_C_CPSR_H

//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CPSR_H
#define ZLVM_C_CPSR_H

#include "Types.h"
#include "State.h"
#include "Value.h"

typedef union CPSR {
    Value value_;
    struct {
        bool N : 1; // negative
        bool Z : 1; // zero
        bool C : 1; // carry
        bool V : 1; // overflow
        bool S : 1; // last operation is signed
        byte _reserved : 3;
        State ST : 8;
        dword _reserverd1 : 48;
    };
} CPSR;

#endif //ZLVM_C_CPSR_H

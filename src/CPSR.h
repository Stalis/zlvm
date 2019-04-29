//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_CPSR_H
#define ZLVM_C_CPSR_H

#include "Value.h"
#include "State.h"

union CPSR {
    union Value value_;
    struct {
        bool N : 1;
        bool Z : 1;
        bool C : 1;
        bool V : 1;
        byte _reserved : 4;
        enum State ST : 8;
        qword _reserverd1 : 48;
    };
};

#endif //ZLVM_C_CPSR_H

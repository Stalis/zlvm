//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_REGISTER_H
#define ZLVM_C_REGISTER_H

#include "Types.h"

union Value {
    byte byte_;
    hword hword_;
    word word_;
//    dword dword_;
};

#endif //ZLVM_C_REGISTER_H

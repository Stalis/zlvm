//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_REGISTER_H
#define ZLVM_C_REGISTER_H

#include "Common.h"

union Value {
    byte byte_;
    hword word_;
    word dword_;
//    qword qword_;
};

#endif //ZLVM_C_REGISTER_H
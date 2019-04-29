//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_COMMON_H
#define ZLVM_C_COMMON_H

#include <cxxabi.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Opcode.h"
#include "State.h"
#include "Condition.h"

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef size_t size_t;

typedef int8_t sbyte;
typedef int16_t sword;
typedef int32_t sdword;
typedef int64_t sqword;

#endif //ZLVM_C_COMMON_H

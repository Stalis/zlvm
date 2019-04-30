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
typedef uint16_t hword;
typedef uint32_t word;
typedef uint64_t dword;

static const byte BYTE_MAX = UINT8_MAX;
static const hword HWORD_MAX = UINT16_MAX;
static const word WORD_MAX = UINT32_MAX;
static const dword DWORD_MAX = UINT64_MAX;

typedef int8_t sbyte;
typedef int16_t shword;
typedef int32_t sword;
typedef int64_t sdword;

static const sbyte SBYTE_MIN = INT8_MIN;
static const shword SHWORD_MIN = INT16_MIN;
static const sword SWORD_MIN = INT32_MIN;
static const sdword SDWORD_MIN = INT64_MIN;

static const sbyte SBYTE_MAX = INT8_MAX;
static const shword SHWORD_MAX = INT16_MAX;
static const sword SWORD_MAX = INT32_MAX;
static const sdword SDWORD_MAX = INT64_MAX;

typedef size_t size_t;

static const size_t SIZE_T_MAX = SIZE_MAX;

#endif //ZLVM_C_COMMON_H

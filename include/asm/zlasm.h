//
// Created by Stanislav on 2019-05-27.
//


#include "Token.h"
#include "LineList.h"

#ifndef ZLVM_C_ZLASM_H
#define ZLVM_C_ZLASM_H

Token** tokenize(char* path, size_t* tokensCount);
Line**  parse(Token** tokens, size_t tokensCount, size_t* linesCount);
Line**  preprocess(Line** lines, size_t currentLinesCount, size_t* linesCount);

uint8_t assemblyLines(Line** lines, size_t linesCount, size_t* binarySize);
uint8_t* assemblySource(char* source, size_t* binarySize);
uint8_t* assemblyFile(char* path, size_t* binarySize);

#endif //ZLVM_C_ZLASM_H

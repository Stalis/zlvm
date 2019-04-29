//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_OPCODE_H
#define ZLVM_C_OPCODE_H

enum Opcode {
    NOP = 0,
    POP, POPR, PUSHR, PUSHI, DUP, // 5
    MOVR, MOVI, ADDR, SUBR, MULR, // 10
    DIVR, MODR, ADDI, SUBI, MULI, // 15
    DIVI, MODI, INT, SYSCALL, JMP, // 20
    NOT, ANDR, ORR, XORR, NANDR, // 25
    NORR, ANDI, ORI, XORI, NANDI, // 30
    NORI, INC, DEC, LOADR, STORER, // 35
    LOADI, STOREI, JMPAL, RET, RESERVED, // 40
    CMPR, CMPI,
    HALT = 0xFF,
};

#endif //ZLVM_C_OPCODE_H

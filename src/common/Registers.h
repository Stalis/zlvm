//
// Created by Stanislav on 2019-05-14.
//

#ifndef ZLVM_C_REGISTERS_H
#define ZLVM_C_REGISTERS_H

enum Registers {
    /* Read-only zero register */
            R_ZERO,
    /* Assembler temporary */
            R_AT,
    /* Procedure results */
            R_V0, R_V1, R_V2, R_V3,
    /* Procedure arguments */
            R_A0, R_A1, R_A2, R_A3,
    /* User temporaries */
            R_T0, R_T1, R_T2, R_T3, R_T4, R_T5, R_T6, R_T7,
    /* User function-across temporaries */
            R_S0, R_S1, R_S2, R_S3, R_S4, R_S5, R_S6, R_S7,
    /* OS Kernel reserved */
            R_K0,
    /* Special registers */
            R_SC, // syscall value
    R_LP, // link pointer
    R_SP, // stack pointer
    R_BP, // base stack pointer
    R_PC, // program counter

    R_TOTAL // total registers
};

#endif //ZLVM_C_REGISTERS_H

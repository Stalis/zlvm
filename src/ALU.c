//
// Created by Stanislav on 2019-04-26.
//

#include "ALU.h"

void alu_compute(struct ALU* alu) {
    switch (alu->op_) {
        case OP_ID:
            alu->result_ = alu->left_;
            break;
        case OP_ADD:
            alu->result_ = alu->left_ + alu->right_;
            break;
        case OP_SUB:
            alu->result_ = alu->left_ - alu->right_;
            break;
        case OP_MUL:
            alu->result_ = alu->left_ * alu->right_;
            break;
        case OP_DIV:
            alu->result_ = alu->left_ / alu->right_;
            break;
        case OP_MOD:
            alu->result_ = alu->left_ % alu->right_;
            break;
        case OP_NOT:
            alu->result_ = ~alu->left_;
            break;
        case OP_AND:
            alu->result_ = alu->left_ & alu->right_;
            break;
        case OP_OR:
            alu->result_ = alu->left_ | alu->right_;
            break;
        case OP_XOR:
            alu->result_ = alu->left_ ^ alu->right_;
            break;
        case OP_NAND:
            alu->result_ = ~(alu->left_ & alu->right_);
            break;
        case OP_NOR:
            alu->result_ = ~(alu->left_ | alu->right_);
            break;
        case OP_INC:
            alu->result_ = alu->left_ + 1;
            break;
        case OP_DEC:
            alu->result_ = alu->left_ - 1;
            break;
        default:
            return;
    }

    alu_setFlags(alu);
}

void alu_setFlags(struct ALU* alu) {
    if (alu->result_ == 0) {
        alu->flags_.Z = true;
    }
    if ((int32_t) alu->result_ < 0) {
        alu->flags_.N = true;
    }
}

void alu_reset(struct ALU* alu) {
    alu->left_ = 0;
    alu->right_ = 0;
    alu->result_ = 0;
    alu->op_ = OP_NOOP;

    byte dummy = 0;
    alu->flags_ = *((struct ALUFlags*) &dummy);
}
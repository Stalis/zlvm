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
        case OP_SADD:
            alu->flags_.S = true;
            alu->result_ = (word) ((sword) alu->left_ + (sword) alu->right_);
            break;
        case OP_SSUB:
            alu->flags_.S = true;
            alu->result_ = (word) ((sword) alu->left_ - (sword) alu->right_);
            break;
        case OP_SMUL:
            alu->flags_.S = true;
            alu->result_ = (word) ((sword) alu->left_ * (sword) alu->right_);
            break;
        case OP_SDIV:
            alu->flags_.S = true;
            alu->result_ = (word) ((sword) alu->left_ / (sword) alu->right_);
            break;
        case OP_SMOD:
            alu->flags_.S = true;
            alu->result_ = (word) ((sword) alu->left_ % (sword) alu->right_);
            break;
        default:
            return;
    }

    alu_setFlags(alu);
}

static inline bool isSignedAddOverflow(sword left, sword right, sword result) {
    if ((left > 0 && right > 0 && result < 0) || (left < 0 && right < 0 && result > 0)) {
        return true;
    }
    return false;
}

static inline bool isSignedMulOverflow(sword left, sword right, sword result) {
    return false;
}

static inline bool isSignedDivOverflow(sword left, sword right, sword result) {
    return false;
}

static inline bool isSignedDivCarry(sword left, sword right, sword result) {
    return false;
}

static void alu_setSignedFlags(struct ALU* alu) {
    sword result = (sword) alu->result_;
    sword left = (sword) alu->left_;
    sword right = (sword) alu->right_;

    switch (alu->op_) {
        case OP_SADD:
            alu->flags_.V = isSignedAddOverflow(left, right, result);
            break;
        case OP_SSUB:
            alu->flags_.V = isSignedAddOverflow(left, -right, result);
            break;
        case OP_SMUL:
            alu->flags_.V = isSignedMulOverflow(left, right, result);
            break;
        case OP_SDIV:
            alu->flags_.V = isSignedDivOverflow(left, right, result);
            alu->flags_.C = isSignedDivCarry(left, right, result);
            break;
        case OP_SMOD:
            break;
        default:
            break;
    }
}

static void alu_setUnsignedFlags(struct ALU* alu) {

}

void alu_setFlags(struct ALU* alu) {
    if (alu->result_ == 0) {
        alu->flags_.Z = true;
    }
    if ((sword) alu->result_ < 0) {
        alu->flags_.N = true;
    }

    if (alu->flags_.S) {
        alu_setSignedFlags(alu);
    } else {
        alu_setUnsignedFlags(alu);
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
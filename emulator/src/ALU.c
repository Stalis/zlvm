// Created by Stanislav on 2019-04-26.
//

#include "ALU.h"

void alu_compute(ALU *alu) {
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
            alu->result_ = (dword)((sdword)alu->left_ + (sdword)alu->right_);
            break;
        case OP_SSUB:
            alu->flags_.S = true;
            alu->result_ = (dword)((sdword)alu->left_ - (sdword)alu->right_);
            break;
        case OP_SMUL:
            alu->flags_.S = true;
            alu->result_ = (dword)((sdword)alu->left_ * (sdword)alu->right_);
            break;
        case OP_SDIV:
            alu->flags_.S = true;
            alu->result_ = (dword)((sdword)alu->left_ / (sdword)alu->right_);
            break;
        case OP_SMOD:
            alu->flags_.S = true;
            alu->result_ = (dword)((sdword)alu->left_ % (sdword)alu->right_);
            break;
        default:
            return;
    }

    alu_setFlags(alu);
}

static bool is_signed_add_overflow(sword left, sword right, sdword result) {
    if ((left > 0 && right > 0 && result < 0) || (left < 0 && right < 0 && result > 0)) {
        return true;
    }
    if (((sdword)left + (sdword)right) != result) {
        return true;
    }
    return false;
}

static bool is_signed_multiply_overflow(sword left, sword right, sdword result) {
    if (((sdword)left * (sdword)right) != result) {
        return true;
    }
    return false;
}

static bool is_signed_divide_overflow(sword left, sword right, sdword result) {
    if (((sdword)left / (sdword)right) != result) {
        return true;
    }
    return false;
}

static void alu_set_signed_flags(ALU *alu) {
    sdword result = (sdword)alu->result_;
    sword left = (sword)alu->left_;
    sword right = (sword)alu->right_;

    switch (alu->op_) {
        case OP_SADD:
            alu->flags_.V = is_signed_add_overflow(left, right, result);
            break;
        case OP_SSUB:
            alu->flags_.V = is_signed_add_overflow(left, -right, result);
            break;
        case OP_SMUL:
            alu->flags_.V = is_signed_multiply_overflow(left, right, result);
            break;
        case OP_SDIV:
            alu->flags_.V = is_signed_divide_overflow(left, right, result);
            break;
        case OP_SMOD:
            break;
        default:
            break;
    }
}

static bool is_unsigned_add_overflow(word left, word right, dword result) {
    if (((dword)left + (dword)right) != result) {
        return true;
    }
    return false;
}

static bool is_unsigned_multiply_overflow(word left, word right, dword result) {
    if (((dword)left * (dword)right) != result) {
        return true;
    }
    return false;
}

static bool is_unsigned_divide_overflow(word left, word right, dword result) {
    if (((dword)left * (dword)right) != result) {
        return true;
    }
    return false;
}

static void alu_set_unsigned_flags(ALU *alu) {
    word left = alu->left_;
    word right = alu->right_;
    dword result = alu->result_;

    switch (alu->op_) {
        case OP_ADD:
            alu->flags_.C = is_unsigned_add_overflow(left, right, result);
            break;
        case OP_SUB:
            alu->flags_.C = is_unsigned_add_overflow(left, ~right, result);
            break;
        case OP_MUL:
            alu->flags_.C = is_unsigned_multiply_overflow(left, right, result);
            break;
        case OP_DIV:
            alu->flags_.C = is_unsigned_divide_overflow(left, right, result);
            break;
        case OP_MOD:
            break;
        default:
            break;
    }
}

void alu_setFlags(ALU *alu) {
    if (alu->result_ == 0) {
        alu->flags_.Z = true;
    }
    if ((sword)alu->result_ < 0) {
        alu->flags_.N = true;
    }

    if (alu->flags_.S) {
        alu_set_signed_flags(alu);
    } else {
        alu_set_unsigned_flags(alu);
    }
}

void alu_reset(ALU *alu) {
    alu->left_ = 0;
    alu->right_ = 0;
    alu->result_ = 0;
    alu->op_ = OP_NOOP;

    alu->flags_ = (ALUFlags){0};
}

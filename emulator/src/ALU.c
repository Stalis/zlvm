// Created by Stanislav on 2019-04-26.
//

#include "ALU.h"

void alu_compute(ALU *alu) {
    alu->flags_ = (ALUFlags){0};

    if ((alu->op_ == OP_DIV || alu->op_ == OP_MOD || alu->op_ == OP_SDIV || alu->op_ == OP_SMOD) &&
        alu->right_ == 0) {
        alu->result_ = 0;
        return;
    }

    switch (alu->op_) {
        case OP_ID:
            alu->result_ = (word)alu->left_;
            break;
        case OP_ADD:
            alu->result_ = (word)(alu->left_ + alu->right_);
            break;
        case OP_SUB:
            alu->result_ = (word)(alu->left_ - alu->right_);
            break;
        case OP_MUL:
            alu->result_ = (word)(alu->left_ * alu->right_);
            break;
        case OP_DIV:
            alu->result_ = (word)(alu->left_ / alu->right_);
            break;
        case OP_MOD:
            alu->result_ = (word)(alu->left_ % alu->right_);
            break;
        case OP_NOT:
            alu->result_ = (word)~alu->left_;
            break;
        case OP_AND:
            alu->result_ = (word)(alu->left_ & alu->right_);
            break;
        case OP_OR:
            alu->result_ = (word)(alu->left_ | alu->right_);
            break;
        case OP_XOR:
            alu->result_ = (word)(alu->left_ ^ alu->right_);
            break;
        case OP_NAND:
            alu->result_ = (word) ~(alu->left_ & alu->right_);
            break;
        case OP_NOR:
            alu->result_ = (word) ~(alu->left_ | alu->right_);
            break;
        case OP_INC:
            alu->result_ = (word)(alu->left_ + 1);
            break;
        case OP_DEC:
            alu->result_ = (word)(alu->left_ - 1);
            break;
        case OP_SADD:
            alu->result_ = (word)((sdword)(sword)alu->left_ + (sdword)(sword)alu->right_);
            break;
        case OP_SSUB:
            alu->result_ = (word)((sdword)(sword)alu->left_ - (sdword)(sword)alu->right_);
            break;
        case OP_SMUL:
            alu->result_ = (word)((sdword)(sword)alu->left_ * (sdword)(sword)alu->right_);
            break;
        case OP_SDIV:
            alu->result_ = (word)((sdword)(sword)alu->left_ / (sdword)(sword)alu->right_);
            break;
        case OP_SMOD:
            alu->result_ = (word)((sdword)(sword)alu->left_ % (sdword)(sword)alu->right_);
            break;
        default:
            return;
    }

    alu_setFlags(alu);
}

static void alu_set_signed_flags(ALU *alu) {
    sword left = (sword)alu->left_;
    sword right = (sword)alu->right_;
    sdword result;

    alu->flags_.S = true;

    switch (alu->op_) {
        case OP_SADD:
            result = (sdword)left + (sdword)right;
            alu->flags_.V = result < SWORD_MIN || result > SWORD_MAX;
            break;
        case OP_SSUB:
            result = (sdword)left - (sdword)right;
            alu->flags_.V = result < SWORD_MIN || result > SWORD_MAX;
            break;
        case OP_SMUL:
            result = (sdword)left * (sdword)right;
            alu->flags_.V = result < SWORD_MIN || result > SWORD_MAX;
            break;
        case OP_SDIV:
            alu->flags_.V = left == SWORD_MIN && right == -1;
            break;
        case OP_SMOD:
            break;
        default:
            break;
    }
}

static void alu_set_unsigned_flags(ALU *alu) {
    word left = alu->left_;
    word right = alu->right_;

    switch (alu->op_) {
        case OP_ADD:
            alu->flags_.C = (dword)left + (dword)right > WORD_MAX;
            break;
        case OP_SUB:
            alu->flags_.C = left >= right;
            break;
        case OP_MUL:
            alu->flags_.C = (dword)left * (dword)right > WORD_MAX;
            break;
        case OP_INC:
            alu->flags_.C = left == WORD_MAX;
            break;
        case OP_DEC:
            alu->flags_.C = left >= 1;
            break;
        default:
            break;
    }
}

void alu_setFlags(ALU *alu) {
    alu->flags_ = (ALUFlags){0};
    alu->flags_.Z = (word)alu->result_ == 0;
    alu->flags_.N = (sword)(word)alu->result_ < 0;

    if (alu->op_ >= OP_SADD && alu->op_ <= OP_SMOD) {
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

#include "emitter.h"

#include <cassert>
#include <cstring>

namespace SenPatcher::x64 {
static void Emit_Instr3Byte_R64_R64(char*& address, R64 dst, R64 src, char instr) {
    char prefix = 0x48;
    if (src >= R64::R8) {
        prefix |= 0x01;
    }
    if (dst >= R64::R8) {
        prefix |= 0x04;
    }

    char op = 0xc0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);

    *address++ = prefix;
    *address++ = instr;
    *address++ = op;
}

void Emit_MOV_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x8b);
}

void Emit_MOV_R64_IMM64(char*& address, R64 dst, uint64_t imm, size_t desiredEncodingLength) {
    if (desiredEncodingLength == 0 && imm == 0) {
        Emit_XOR_R64_R64(address, dst, dst);
    } else if (desiredEncodingLength == 0 && imm < 0x1'0000'0000) {
        char op = 0xb8;
        op |= (static_cast<int>(dst) & 0x7);

        if (dst >= R64::R8) {
            *address++ = 0x41;
        }
        *address++ = op;
        uint32_t imm32 = static_cast<uint32_t>(imm);
        std::memcpy(address, &imm32, 4);
        address += 4;
    } else if ((desiredEncodingLength == 0 || desiredEncodingLength == 7)
               && (imm < 0x8000'0000 || imm >= 0xffff'ffff'8000'0000)) {
        char prefix = 0x48;
        if (dst >= R64::R8) {
            prefix |= 0x01;
        }

        char op = 0xc0;
        op |= (static_cast<int>(dst) & 0x7);

        *address++ = prefix;
        *address++ = 0xc7;
        *address++ = op;
        uint32_t imm32 = static_cast<uint32_t>(imm & 0xffff'ffff);
        std::memcpy(address, &imm32, 4);
        address += 4;
    } else {
        char prefix = 0x48;
        if (dst >= R64::R8) {
            prefix |= 0x01;
        }

        char op = 0xb8;
        op |= (static_cast<int>(dst) & 0x7);

        *address++ = prefix;
        *address++ = op;
        std::memcpy(address, &imm, 8);
        address += 8;
    }
}

void Emit_JMP_R64(char*& address, R64 target) {
    char op = 0xe0;
    op |= (static_cast<int>(target) & 0x7);

    if (target >= R64::R8) {
        *address++ = 0x41;
    }
    *address++ = 0xff;
    *address++ = op;
}

void Emit_CALL_R64(char*& address, R64 target) {
    char op = 0xd0;
    op |= (static_cast<int>(target) & 0x7);

    if (target >= R64::R8) {
        *address++ = 0x41;
    }
    *address++ = 0xff;
    *address++ = op;
}

void Emit_RET(char*& address) {
    *address++ = 0xc3;
}

void Emit_PUSH_R64(char*& address, R64 reg) {
    char op = 0x50;
    op |= (static_cast<int>(reg) & 0x7);

    if (reg >= R64::R8) {
        *address++ = 0x41;
    }
    *address++ = op;
}

void Emit_POP_R64(char*& address, R64 reg) {
    char op = 0x58;
    op |= (static_cast<int>(reg) & 0x7);

    if (reg >= R64::R8) {
        *address++ = 0x41;
    }
    *address++ = op;
}

void Emit_ADD_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x03);
}

void Emit_ADC_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x13);
}

void Emit_SUB_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x2b);
}

void Emit_TEST_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x85);
}

void Emit_CMP_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x3b);
}

void Emit_AND_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x23);
}

void Emit_OR_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, 0x0b);
}

void Emit_XOR_R64_R64(char*& address, R64 dst, R64 src) {
    if (dst == src && dst < R64::R8) {
        char op = 0xc0;
        op |= (static_cast<int>(src) & 0x7);
        op |= ((static_cast<int>(dst) & 0x7) << 3);
        *address++ = 0x33;
        *address++ = op;
    } else {
        Emit_Instr3Byte_R64_R64(address, dst, src, 0x33);
    }
}

void Emit_ADD_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RAX, dst, 0x83);
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = 0x48;
        *address++ = 0x05;
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RAX, dst, 0x81);
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_ADC_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RDX, dst, 0x83);
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = 0x48;
        *address++ = 0x15;
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RDX, dst, 0x81);
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_SUB_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RBP, dst, 0x83);
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = 0x48;
        *address++ = 0x2d;
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RBP, dst, 0x81);
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void BranchHelper1Byte::SetTarget(char* target) {
    assert(Target == nullptr);
    Target = target;
    if (!Sources.empty()) {
        for (char* s : Sources) {
            Commit(s);
        }
        Sources.clear();
    }
}

void BranchHelper1Byte::AddSource(char* source) {
    if (Target == nullptr) {
        Sources.push_back(source);
    } else {
        Commit(source);
    }
}

void BranchHelper1Byte::Commit(char* source) {
    int64_t diff = (Target - (source + 1));
    assert(diff >= -128 && diff <= 127);
    *source = static_cast<char>(diff);
}

void BranchHelper1Byte::WriteJump(char*& address, JumpCondition condition) {
    assert(condition != JumpCondition::CALL);
    if (condition == JumpCondition::JMP) {
        *address++ = 0xeb;
    } else {
        char op = 0x70 | static_cast<int>(condition);
        *address++ = op;
    }
    *address++ = 0xfe; // self-jump until set
    AddSource(address - 1);
}
} // namespace SenPatcher::x64

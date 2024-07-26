#include "emitter.h"

#include <cassert>
#include <cstring>

namespace SenPatcher::x64 {
static void Emit_Instr3Byte_R64_R64(char*& address, R64 dst, R64 src, char instr) {
    int prefix = 0x48;
    if (src >= R64::R8) {
        prefix |= 0x01;
    }
    if (dst >= R64::R8) {
        prefix |= 0x04;
    }

    int op = 0xc0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);

    *address++ = static_cast<char>(prefix);
    *address++ = instr;
    *address++ = static_cast<char>(op);
}

void Emit_MOV_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x8b));
}

void Emit_MOV_R64_IMM64(char*& address, R64 dst, uint64_t imm, size_t desiredEncodingLength) {
    if (desiredEncodingLength == 0 && imm == 0) {
        Emit_XOR_R64_R64(address, dst, dst);
    } else if (desiredEncodingLength == 0 && imm < 0x1'0000'0000) {
        int op = 0xb8;
        op |= (static_cast<int>(dst) & 0x7);

        if (dst >= R64::R8) {
            *address++ = static_cast<char>(0x41);
        }
        *address++ = static_cast<char>(op);
        uint32_t imm32 = static_cast<uint32_t>(imm);
        std::memcpy(address, &imm32, 4);
        address += 4;
    } else if ((desiredEncodingLength == 0 || desiredEncodingLength == 7)
               && (imm < 0x8000'0000 || imm >= 0xffff'ffff'8000'0000)) {
        int prefix = 0x48;
        if (dst >= R64::R8) {
            prefix |= 0x01;
        }

        int op = 0xc0;
        op |= (static_cast<int>(dst) & 0x7);

        *address++ = static_cast<char>(prefix);
        *address++ = static_cast<char>(0xc7);
        *address++ = static_cast<char>(op);
        uint32_t imm32 = static_cast<uint32_t>(imm & 0xffff'ffff);
        std::memcpy(address, &imm32, 4);
        address += 4;
    } else {
        int prefix = 0x48;
        if (dst >= R64::R8) {
            prefix |= 0x01;
        }

        int op = 0xb8;
        op |= (static_cast<int>(dst) & 0x7);

        *address++ = static_cast<char>(prefix);
        *address++ = static_cast<char>(op);
        std::memcpy(address, &imm, 8);
        address += 8;
    }
}

void Emit_MOVZX_R32_BytePtrR64(char*& address, R32 dst, R64 src) {
    if (dst >= R32::R8D || src >= R64::R8) {
        int prefix = 0x40;
        if (src >= R64::R8) {
            prefix |= 1;
        }
        if (dst >= R32::R8D) {
            prefix |= 4;
        }
        *address++ = static_cast<char>(prefix);
    }
    *address++ = static_cast<char>(0x0f);
    *address++ = static_cast<char>(0xb6);

    int op = (src == R64::RBP || src == R64::R13) ? 0x40 : 0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);
    *address++ = static_cast<char>(op);
    if (src == R64::RSP || src == R64::R12) {
        *address++ = static_cast<char>(0x24);
    } else if (src == R64::RBP || src == R64::R13) {
        *address++ = static_cast<char>(0);
    }
}

void Emit_MOVZX_R32_R8(char*& address, R32 dst, R8 src) {
    if (dst >= R32::R8D || src >= R8::R8B) {
        int prefix = 0x40;
        if (src >= R8::R8B) {
            prefix |= 1;
        }
        if (dst >= R32::R8D) {
            prefix |= 4;
        }
        *address++ = static_cast<char>(prefix);
    }
    *address++ = static_cast<char>(0x0f);
    *address++ = static_cast<char>(0xb6);

    int op = 0xc0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);
    *address++ = static_cast<char>(op);
}

void Emit_JMP_R64(char*& address, R64 target) {
    int op = 0xe0;
    op |= (static_cast<int>(target) & 0x7);

    if (target >= R64::R8) {
        *address++ = static_cast<char>(0x41);
    }
    *address++ = static_cast<char>(0xff);
    *address++ = static_cast<char>(op);
}

void Emit_CALL_R64(char*& address, R64 target) {
    int op = 0xd0;
    op |= (static_cast<int>(target) & 0x7);

    if (target >= R64::R8) {
        *address++ = static_cast<char>(0x41);
    }
    *address++ = static_cast<char>(0xff);
    *address++ = static_cast<char>(op);
}

void Emit_RET(char*& address) {
    *address++ = static_cast<char>(0xc3);
}

void Emit_PUSH_R64(char*& address, R64 reg) {
    int op = 0x50;
    op |= (static_cast<int>(reg) & 0x7);

    if (reg >= R64::R8) {
        *address++ = static_cast<char>(0x41);
    }
    *address++ = static_cast<char>(op);
}

void Emit_POP_R64(char*& address, R64 reg) {
    int op = 0x58;
    op |= (static_cast<int>(reg) & 0x7);

    if (reg >= R64::R8) {
        *address++ = static_cast<char>(0x41);
    }
    *address++ = static_cast<char>(op);
}

void Emit_ADD_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x03));
}

void Emit_ADC_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x13));
}

void Emit_SUB_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x2b));
}

void Emit_TEST_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x85));
}

void Emit_CMP_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x3b));
}

void Emit_AND_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x23));
}

void Emit_OR_R64_R64(char*& address, R64 dst, R64 src) {
    Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x0b));
}

void Emit_XOR_R64_R64(char*& address, R64 dst, R64 src) {
    if (dst == src && dst < R64::R8) {
        int op = 0xc0;
        op |= (static_cast<int>(src) & 0x7);
        op |= ((static_cast<int>(dst) & 0x7) << 3);
        *address++ = static_cast<char>(0x33);
        *address++ = static_cast<char>(op);
    } else {
        Emit_Instr3Byte_R64_R64(address, dst, src, static_cast<char>(0x33));
    }
}

void Emit_ADD_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RAX, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = static_cast<char>(0x48);
        *address++ = static_cast<char>(0x05);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RAX, dst, static_cast<char>(0x81));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_ADC_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RDX, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = 0x48;
        *address++ = 0x15;
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RDX, dst, static_cast<char>(0x81));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_SUB_R64_IMM32(char*& address, R64 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr3Byte_R64_R64(address, R64::RBP, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R64::RAX) {
        *address++ = static_cast<char>(0x48);
        *address++ = static_cast<char>(0x2d);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr3Byte_R64_R64(address, R64::RBP, dst, static_cast<char>(0x81));
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
        *address++ = static_cast<char>(0xeb);
    } else {
        char op = static_cast<char>(0x70 | static_cast<int>(condition));
        *address++ = op;
    }
    *address++ = static_cast<char>(0xfe); // self-jump until set
    AddSource(address - 1);
}

void BranchHelper4Byte::SetTarget(char* target) {
    assert(Target == nullptr);
    Target = target;
    if (!Sources.empty()) {
        for (char* s : Sources) {
            Commit(s);
        }
        Sources.clear();
    }
}

void BranchHelper4Byte::AddSource(char* source) {
    if (Target == nullptr) {
        Sources.push_back(source);
    } else {
        Commit(source);
    }
}

void BranchHelper4Byte::Commit(char* source) {
    int64_t diff = (Target - (source + 4));
    assert(diff >= INT32_MIN && diff <= INT32_MAX);
    int32_t diff32 = static_cast<int32_t>(diff);
    std::memcpy(source, &diff32, 4);
}

void BranchHelper4Byte::WriteJump(char*& address, JumpCondition condition) {
    int32_t offset;
    if (condition == JumpCondition::JMP) {
        *address++ = static_cast<char>(0xe9);
        offset = -5;
    } else if (condition == JumpCondition::CALL) {
        *address++ = static_cast<char>(0xe8);
        offset = -5;
    } else {
        *address++ = static_cast<char>(0x0f);
        char op = static_cast<char>(0x80 | static_cast<int>(condition));
        *address++ = op;
        offset = -6;
    }
    std::memcpy(address, &offset, 4); // self-jump until set
    AddSource(address);
    address += 4;
}
} // namespace SenPatcher::x64

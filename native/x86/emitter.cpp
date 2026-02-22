#include "emitter.h"

#include <cassert>
#include <cstring>

namespace SenPatcher::x86 {
static void Emit_Instr2Byte_R32_R32(char*& address, R32 dst, R32 src, char instr) {
    int op = 0xc0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);

    *address++ = instr;
    *address++ = static_cast<char>(op);
}

static void Emit_Instr2Byte_R8_R8(char*& address, R8 dst, R8 src, char instr) {
    int op = 0xc0;
    op |= (static_cast<int>(src) & 0x7);
    op |= ((static_cast<int>(dst) & 0x7) << 3);

    *address++ = instr;
    *address++ = static_cast<char>(op);
}

void Emit_MOV_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x8b));
}

void Emit_MOV_R32_IMM32(char*& address, R32 dst, uint32_t imm, size_t desiredEncodingLength) {
    if (desiredEncodingLength == 0 && imm == 0) {
        Emit_XOR_R32_R32(address, dst, dst);
    } else {
        int op = 0xb8;
        op |= (static_cast<int>(dst) & 0x7);
        *address++ = static_cast<char>(op);
        uint32_t imm32 = static_cast<uint32_t>(imm);
        std::memcpy(address, &imm32, 4);
        address += 4;
    }
}

void Emit_MOV_R32_DwordPtr(char*& address, R32 dst, void* src) {
    if (dst == R32::EAX) {
        *address++ = static_cast<char>(0xa1);
    } else {
        *address++ = static_cast<char>(0x8b);
        *address++ = static_cast<char>(0x05 | ((static_cast<int>(dst) & 0x7) << 3));
    }
    std::memcpy(address, &src, 4);
    address += 4;
}

void Emit_MOV_DwordPtr_R32(char*& address, void* dst, R32 src) {
    if (src == R32::EAX) {
        *address++ = static_cast<char>(0xa3);
    } else {
        *address++ = static_cast<char>(0x89);
        *address++ = static_cast<char>(0x05 | ((static_cast<int>(src) & 0x7) << 3));
    }
    std::memcpy(address, &dst, 4);
    address += 4;
}

void Emit_MOV_R32_DwordPtrR32PlusOffset(char*& address, R32 dst, R32 src, int32_t offset) {
    if (offset >= -128 && offset <= 127) {
        int op = 0x40;
        op |= (static_cast<int>(src) & 0x7);
        op |= ((static_cast<int>(dst) & 0x7) << 3);

        *address++ = static_cast<char>(0x8b);
        *address++ = static_cast<char>(op);
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(offset);
    } else {
        int op = 0x80;
        op |= (static_cast<int>(src) & 0x7);
        op |= ((static_cast<int>(dst) & 0x7) << 3);

        *address++ = static_cast<char>(0x8b);
        *address++ = static_cast<char>(op);
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(offset);
        std::memcpy(address, &offset, 4);
        address += 4;
    }
}

void Emit_MOV_DwordPtrR32PlusOffset_R32(char*& address, R32 dst, int32_t offset, R32 src) {
    if (offset >= -128 && offset <= 127) {
        int op = 0x40;
        op |= (static_cast<int>(dst) & 0x7);
        op |= ((static_cast<int>(src) & 0x7) << 3);

        *address++ = static_cast<char>(0x89);
        *address++ = static_cast<char>(op);
        if (dst == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(offset);
    } else {
        int op = 0x80;
        op |= (static_cast<int>(dst) & 0x7);
        op |= ((static_cast<int>(src) & 0x7) << 3);

        *address++ = static_cast<char>(0x89);
        *address++ = static_cast<char>(op);
        if (dst == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(offset);
        std::memcpy(address, &offset, 4);
        address += 4;
    }
}

void Emit_MOV_R8_IMM8(char*& address, R8 dst, uint8_t imm) {
    int op = 0xb0 | (static_cast<int>(dst) & 0x7);
    *address++ = static_cast<char>(op);
    *address++ = static_cast<char>(imm);
}

void Emit_MOV_R8_BytePtr(char*& address, R8 dst, void* src) {
    if (dst == R8::AL) {
        *address++ = static_cast<char>(0xa0);
    } else {
        *address++ = static_cast<char>(0x8a);
        *address++ = static_cast<char>(0x5 | ((static_cast<int>(dst) & 0x7) << 3));
    }

    uint32_t imm32 = reinterpret_cast<uint32_t>(src);
    std::memcpy(address, &imm32, 4);
    address += 4;
}

void Emit_MOV_BytePtr_R8(char*& address, void* dst, R8 src) {
    if (src == R8::AL) {
        *address++ = static_cast<char>(0xa2);
    } else {
        *address++ = static_cast<char>(0x88);
        *address++ = static_cast<char>(0x5 | ((static_cast<int>(src) & 0x7) << 3));
    }

    uint32_t imm32 = reinterpret_cast<uint32_t>(dst);
    std::memcpy(address, &imm32, 4);
    address += 4;
}

void Emit_JMP_R32(char*& address, R32 target) {
    int op = 0xe0;
    op |= (static_cast<int>(target) & 0x7);
    *address++ = static_cast<char>(0xff);
    *address++ = static_cast<char>(op);
}

void Emit_CALL_R32(char*& address, R32 target) {
    int op = 0xd0;
    op |= (static_cast<int>(target) & 0x7);
    *address++ = static_cast<char>(0xff);
    *address++ = static_cast<char>(op);
}

void Emit_RET(char*& address) {
    *address++ = static_cast<char>(0xc3);
}

void Emit_RET_IMM16(char*& address, uint16_t imm) {
    *address++ = static_cast<char>(0xc2);
    std::memcpy(address, &imm, 2);
    address += 2;
}

void Emit_PUSH_R32(char*& address, R32 reg) {
    int op = 0x50;
    op |= (static_cast<int>(reg) & 0x7);
    *address++ = static_cast<char>(op);
}

void Emit_POP_R32(char*& address, R32 reg) {
    int op = 0x58;
    op |= (static_cast<int>(reg) & 0x7);
    *address++ = static_cast<char>(op);
}

void Emit_ADD_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x03));
}

void Emit_ADC_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x13));
}

void Emit_SUB_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x2b));
}

void Emit_TEST_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x85));
}

void Emit_TEST_R8_R8(char*& address, R8 dst, R8 src) {
    // yes this is flipped, i don't know why
    Emit_Instr2Byte_R8_R8(address, src, dst, static_cast<char>(0x84));
}

void Emit_TEST_R8_BytePtr(char*& address, R8 dst, void* src) {
    *address++ = static_cast<char>(0x84);
    *address++ = static_cast<char>(0x5 | ((static_cast<int>(dst) & 0x7) << 3));
    uint32_t imm32 = reinterpret_cast<uint32_t>(src);
    std::memcpy(address, &imm32, 4);
    address += 4;
}

void Emit_CMP_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x3b));
}

void Emit_CMP_R32_IMM32(char*& address, R32 dst, int32_t imm) {
    if (imm >= -0x80 && imm <= 0x7f) {
        *address++ = static_cast<char>(0x83);
        *address++ = static_cast<char>(0xf8 | (static_cast<int>(dst) & 0x7));
        *address++ = static_cast<char>(static_cast<int8_t>(imm));
    } else if (dst == R32::EAX) {
        *address++ = static_cast<char>(0x3d);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        *address++ = static_cast<char>(0x81);
        *address++ = static_cast<char>(0xf8 | (static_cast<int>(dst) & 0x7));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_CMP_R8_R8(char*& address, R8 dst, R8 src) {
    Emit_Instr2Byte_R8_R8(address, dst, src, static_cast<char>(0x3a));
}

void Emit_AND_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x23));
}

void Emit_OR_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x0b));
}

void Emit_XOR_R32_R32(char*& address, R32 dst, R32 src) {
    Emit_Instr2Byte_R32_R32(address, dst, src, static_cast<char>(0x33));
}

void Emit_ADD_R32_IMM32(char*& address, R32 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr2Byte_R32_R32(address, R32::EAX, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R32::EAX) {
        *address++ = static_cast<char>(0x05);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr2Byte_R32_R32(address, R32::EAX, dst, static_cast<char>(0x81));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_ADC_R32_IMM32(char*& address, R32 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr2Byte_R32_R32(address, R32::EDX, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R32::EAX) {
        *address++ = static_cast<char>(0x15);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr2Byte_R32_R32(address, R32::EDX, dst, static_cast<char>(0x81));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_SUB_R32_IMM32(char*& address, R32 dst, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        Emit_Instr2Byte_R32_R32(address, R32::EBP, dst, static_cast<char>(0x83));
        *address++ = static_cast<char>(imm);
    } else if (dst == R32::EAX) {
        *address++ = static_cast<char>(0x2d);
        std::memcpy(address, &imm, 4);
        address += 4;
    } else {
        Emit_Instr2Byte_R32_R32(address, R32::EBP, dst, static_cast<char>(0x81));
        std::memcpy(address, &imm, 4);
        address += 4;
    }
}

void Emit_FLD_DwordPtr(char*& address, const void* src) {
    *address++ = static_cast<char>(0xd9);
    *address++ = static_cast<char>(0x05);
    std::memcpy(address, &src, 4);
    address += 4;
}

void Emit_FLD_DwordPtrR32PlusOffset(char*& address, R32 src, int32_t offset) {
    if (offset >= -128 && offset <= 127) {
        *address++ = static_cast<char>(0xd9);
        *address++ = static_cast<char>(0x40 | (static_cast<int>(src) & 0x7));
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(static_cast<int8_t>(offset));
    } else {
        *address++ = static_cast<char>(0xd9);
        *address++ = static_cast<char>(0x80 | (static_cast<int>(src) & 0x7));
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        std::memcpy(address, &offset, 4);
        address += 4;
    }
}

void Emit_FILD_DwordPtrR32PlusOffset(char*& address, R32 src, int32_t offset) {
    if (offset >= -128 && offset <= 127) {
        *address++ = static_cast<char>(0xdb);
        *address++ = static_cast<char>(0x40 | (static_cast<int>(src) & 0x7));
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        *address++ = static_cast<char>(static_cast<int8_t>(offset));
    } else {
        *address++ = static_cast<char>(0xdb);
        *address++ = static_cast<char>(0x80 | (static_cast<int>(src) & 0x7));
        if (src == R32::ESP) {
            *address++ = static_cast<char>(0x24);
        }
        std::memcpy(address, &offset, 4);
        address += 4;
    }
}

void Emit_FADD_DwordPtr(char*& address, const void* src) {
    *address++ = static_cast<char>(0xd8);
    *address++ = static_cast<char>(0x05);
    std::memcpy(address, &src, 4);
    address += 4;
}

void Emit_FSUB_DwordPtr(char*& address, const void* src) {
    *address++ = static_cast<char>(0xd8);
    *address++ = static_cast<char>(0x25);
    std::memcpy(address, &src, 4);
    address += 4;
}

void Emit_FMUL_DwordPtr(char*& address, const void* src) {
    *address++ = static_cast<char>(0xd8);
    *address++ = static_cast<char>(0x0d);
    std::memcpy(address, &src, 4);
    address += 4;
}

void Emit_FDIV_DwordPtr(char*& address, const void* src) {
    *address++ = static_cast<char>(0xd8);
    *address++ = static_cast<char>(0x35);
    std::memcpy(address, &src, 4);
    address += 4;
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
    int32_t diff = (Target - (source + 1));
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
    int32_t diff = (Target - (source + 4));
    std::memcpy(source, &diff, 4);
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
} // namespace SenPatcher::x86

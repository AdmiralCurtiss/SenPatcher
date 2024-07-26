#pragma once

#include <cstdint>
#include <vector>

namespace SenPatcher::x64 {
enum class R8 {
    AL = 0,
    CL = 1,
    DL = 2,
    BL = 3,
    R8B = 8,
    R9B = 9,
    R10B = 10,
    R11B = 11,
    R12B = 12,
    R13B = 13,
    R14B = 14,
    R15B = 15,
};
enum class R16 {
    AX = 0,
    CX = 1,
    DX = 2,
    BX = 3,
    SP = 4,
    BP = 5,
    SI = 6,
    DI = 7,
    R8W = 8,
    R9W = 9,
    R10W = 10,
    R11W = 11,
    R12W = 12,
    R13W = 13,
    R14W = 14,
    R15W = 15,
};
enum class R32 {
    EAX = 0,
    ECX = 1,
    EDX = 2,
    EBX = 3,
    ESP = 4,
    EBP = 5,
    ESI = 6,
    EDI = 7,
    R8D = 8,
    R9D = 9,
    R10D = 10,
    R11D = 11,
    R12D = 12,
    R13D = 13,
    R14D = 14,
    R15D = 15,
};
enum class R64 {
    RAX = 0,
    RCX = 1,
    RDX = 2,
    RBX = 3,
    RSP = 4,
    RBP = 5,
    RSI = 6,
    RDI = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
};
enum class JumpCondition {
    JA = 0x7,
    JAE = 0x3,
    JB = 0x2,
    JBE = 0x6,
    JC = 0x2,
    JE = 0x4,
    JG = 0xf,
    JGE = 0xd,
    JL = 0xc,
    JLE = 0xe,
    JNA = 0x6,
    JNAE = 0x2,
    JNB = 0x3,
    JNBE = 0x7,
    JNC = 0x3,
    JNE = 0x5,
    JNG = 0xe,
    JNGE = 0xc,
    JNL = 0xd,
    JNLE = 0xf,
    JNO = 0x1,
    JNP = 0xb,
    JNS = 0x9,
    JNZ = 0x5,
    JO = 0x0,
    JP = 0xa,
    JPE = 0xa,
    JPO = 0xb,
    JS = 0x8,
    JZ = 0x4,

    JMP = 0x10,
    CALL = 0x11,
};

void Emit_MOV_R64_R64(char*& address, R64 dst, R64 src);
void Emit_MOV_R64_IMM64(char*& address, R64 dst, uint64_t imm, size_t desiredEncodingLength = 0);
void Emit_MOVZX_R32_BytePtrR64(char*& address, R32 dst, R64 src);
void Emit_MOVZX_R32_R8(char*& address, R32 dst, R8 src);
void Emit_JMP_R64(char*& address, R64 target);
void Emit_CALL_R64(char*& address, R64 target);
void Emit_RET(char*& address);
void Emit_PUSH_R64(char*& address, R64 reg);
void Emit_POP_R64(char*& address, R64 reg);
void Emit_ADD_R64_R64(char*& address, R64 dst, R64 src);
void Emit_ADC_R64_R64(char*& address, R64 dst, R64 src);
void Emit_SUB_R64_R64(char*& address, R64 dst, R64 src);
void Emit_TEST_R64_R64(char*& address, R64 dst, R64 src);
void Emit_CMP_R64_R64(char*& address, R64 dst, R64 src);
void Emit_AND_R64_R64(char*& address, R64 dst, R64 src);
void Emit_OR_R64_R64(char*& address, R64 dst, R64 src);
void Emit_XOR_R64_R64(char*& address, R64 dst, R64 src);
void Emit_ADD_R64_IMM32(char*& address, R64 dst, int32_t imm);
void Emit_ADC_R64_IMM32(char*& address, R64 dst, int32_t imm);
void Emit_SUB_R64_IMM32(char*& address, R64 dst, int32_t imm);

struct BranchHelper1Byte {
public:
    BranchHelper1Byte() = default;
    BranchHelper1Byte(const BranchHelper1Byte& other) = delete;
    BranchHelper1Byte(BranchHelper1Byte&& other) = delete;
    BranchHelper1Byte& operator=(const BranchHelper1Byte& other) = delete;
    BranchHelper1Byte& operator=(BranchHelper1Byte&& other) = delete;
    ~BranchHelper1Byte() = default;

    void SetTarget(char* target);
    void WriteJump(char*& address, JumpCondition condition);

private:
    char* Target = nullptr;
    std::vector<char*> Sources;

    void AddSource(char* source);
    void Commit(char* source);
};

struct BranchHelper4Byte {
public:
    BranchHelper4Byte() = default;
    BranchHelper4Byte(const BranchHelper4Byte& other) = delete;
    BranchHelper4Byte(BranchHelper4Byte&& other) = delete;
    BranchHelper4Byte& operator=(const BranchHelper4Byte& other) = delete;
    BranchHelper4Byte& operator=(BranchHelper4Byte&& other) = delete;
    ~BranchHelper4Byte() = default;

    void SetTarget(char* target);

    // CALL and JMP result in 5 bytes, everything else is 6 bytes
    void WriteJump(char*& address, JumpCondition condition);

private:
    char* Target = nullptr;
    std::vector<char*> Sources;

    void AddSource(char* source);
    void Commit(char* source);
};

inline void WriteInstruction48(char*& codepos, uint64_t instr) {
    *codepos++ = (char)((instr >> 40) & 0xff);
    *codepos++ = (char)((instr >> 32) & 0xff);
    *codepos++ = (char)((instr >> 24) & 0xff);
    *codepos++ = (char)((instr >> 16) & 0xff);
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

inline void WriteInstruction40(char*& codepos, uint64_t instr) {
    *codepos++ = (char)((instr >> 32) & 0xff);
    *codepos++ = (char)((instr >> 24) & 0xff);
    *codepos++ = (char)((instr >> 16) & 0xff);
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

inline void WriteInstruction32(char*& codepos, uint32_t instr) {
    *codepos++ = (char)((instr >> 24) & 0xff);
    *codepos++ = (char)((instr >> 16) & 0xff);
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

inline void WriteInstruction24(char*& codepos, uint32_t instr) {
    *codepos++ = (char)((instr >> 16) & 0xff);
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

inline void WriteInstruction16(char*& codepos, uint32_t instr) {
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

inline void WriteInstruction8(char*& codepos, uint32_t instr) {
    *codepos++ = (char)(instr & 0xff);
}
} // namespace SenPatcher::x64

#pragma once

#include <cstdint>
#include <vector>

namespace SenPatcher::x86 {
enum class R8 {
    AL = 0,
    CL = 1,
    DL = 2,
    BL = 3,
    AH = 4,
    CH = 5,
    DH = 6,
    BH = 7,
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

void Emit_MOV_R32_R32(char*& address, R32 dst, R32 src);
void Emit_MOV_R32_IMM32(char*& address, R32 dst, uint32_t imm, size_t desiredEncodingLength = 0);
void Emit_MOV_R32_PtrR32PlusOffset8(char*& address, R32 dst, R32 src, int8_t offset);
void Emit_MOV_R8_IMM8(char*& address, R8 dst, uint8_t imm);
void Emit_MOV_R8_BytePtr(char*& address, R8 dst, void* src);
void Emit_MOV_BytePtr_R8(char*& address, void* dst, R8 src);
void Emit_JMP_R32(char*& address, R32 target);
void Emit_CALL_R32(char*& address, R32 target);
void Emit_RET(char*& address);
void Emit_RET_IMM16(char*& address, uint16_t imm);
void Emit_PUSH_R32(char*& address, R32 reg);
void Emit_POP_R32(char*& address, R32 reg);
void Emit_ADD_R32_R32(char*& address, R32 dst, R32 src);
void Emit_ADC_R32_R32(char*& address, R32 dst, R32 src);
void Emit_SUB_R32_R32(char*& address, R32 dst, R32 src);
void Emit_TEST_R32_R32(char*& address, R32 dst, R32 src);
void Emit_TEST_R8_R8(char*& address, R8 dst, R8 src);
void Emit_TEST_R8_BytePtr(char*& address, R8 dst, void* src);
void Emit_CMP_R32_R32(char*& address, R32 dst, R32 src);
void Emit_CMP_R8_R8(char*& address, R8 dst, R8 src);
void Emit_AND_R32_R32(char*& address, R32 dst, R32 src);
void Emit_OR_R32_R32(char*& address, R32 dst, R32 src);
void Emit_XOR_R32_R32(char*& address, R32 dst, R32 src);
void Emit_ADD_R32_IMM32(char*& address, R32 dst, int32_t imm);
void Emit_ADC_R32_IMM32(char*& address, R32 dst, int32_t imm);
void Emit_SUB_R32_IMM32(char*& address, R32 dst, int32_t imm);

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

inline void WriteInstruction56(char*& codepos, uint64_t instr) {
    *codepos++ = (char)((instr >> 48) & 0xff);
    *codepos++ = (char)((instr >> 40) & 0xff);
    *codepos++ = (char)((instr >> 32) & 0xff);
    *codepos++ = (char)((instr >> 24) & 0xff);
    *codepos++ = (char)((instr >> 16) & 0xff);
    *codepos++ = (char)((instr >> 8) & 0xff);
    *codepos++ = (char)(instr & 0xff);
}

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
} // namespace SenPatcher::x86

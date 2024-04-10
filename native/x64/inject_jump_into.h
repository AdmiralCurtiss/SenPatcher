#pragma once

#include <array>
#include <bit>
#include <cstring>

#include "logger.h"
#include "x64/emitter.h"
#include "x64/page_unprotect.h"

namespace SenPatcher::x64 {
template<size_t length>
struct InjectJumpIntoCodeResult {
    char* JumpBackAddress;
    std::array<char, length> OverwrittenInstructions;
};

enum class PaddingInstruction { Interrupt, Nop };
template<PaddingInstruction paddingInstruction>
constexpr bool IsValidPaddingInstruction = paddingInstruction == PaddingInstruction::Interrupt
                                           || paddingInstruction == PaddingInstruction::Nop;

template<size_t length, PaddingInstruction paddingInstruction = PaddingInstruction::Interrupt>
static InjectJumpIntoCodeResult<length> InjectJumpIntoCode(SenPatcher::Logger& logger,
                                                           char* injectAt,
                                                           SenPatcher::x64::R64 reg,
                                                           char* jumpTarget) {
    static_assert(length >= 12);

    InjectJumpIntoCodeResult<length> rv;
    char* inject = injectAt;
    {
        PageUnprotect page(logger, inject, rv.OverwrittenInstructions.size());
        std::memcpy(rv.OverwrittenInstructions.data(), inject, rv.OverwrittenInstructions.size());

        Emit_MOV_R64_IMM64(inject, reg, std::bit_cast<uint64_t>(jumpTarget), 10);
        Emit_JMP_R64(inject, reg);
        if constexpr (length > 12) {
            static_assert(IsValidPaddingInstruction<paddingInstruction>);
            if constexpr (paddingInstruction == PaddingInstruction::Interrupt) {
                for (size_t i = 12; i < length; ++i) {
                    *inject++ = static_cast<char>(0xcc);
                }
            } else if constexpr (paddingInstruction == PaddingInstruction::Nop) {
                for (size_t i = 12; i < length; ++i) {
                    *inject++ = static_cast<char>(0x90);
                }
            }
        }
    }
    rv.JumpBackAddress = inject;
    return rv;
}

// Intended usage: When you want to inject but can't find 12 bytes of overwritable code. This way we
// can get away with just 5. Step 1 injects a relative jmp to Step 2, Step 2 does the standard mov
// rax, jmp rax. Typically your Step 2 will be somewhere in the between-function padding.
template<size_t lengthStep1,
         size_t lengthStep2,
         PaddingInstruction paddingInstructionStep1 = PaddingInstruction::Interrupt,
         PaddingInstruction paddingInstructionStep2 = PaddingInstruction::Interrupt>
static InjectJumpIntoCodeResult<lengthStep1> InjectJumpIntoCode2Step(SenPatcher::Logger& logger,
                                                                     char* injectAtStep1,
                                                                     char* injectAtStep2,
                                                                     SenPatcher::x64::R64 reg,
                                                                     char* jumpTarget) {
    InjectJumpIntoCode<lengthStep2, paddingInstructionStep2>(
        logger, injectAtStep2, reg, jumpTarget);

    static_assert(lengthStep1 >= 5);

    InjectJumpIntoCodeResult<lengthStep1> rv;
    {
        PageUnprotect page(logger, injectAtStep1, rv.OverwrittenInstructions.size());
        std::memcpy(
            rv.OverwrittenInstructions.data(), injectAtStep1, rv.OverwrittenInstructions.size());

        BranchHelper4Byte branch;
        branch.SetTarget(injectAtStep2);
        branch.WriteJump(injectAtStep1, JumpCondition::JMP);
        if constexpr (lengthStep1 > 5) {
            static_assert(IsValidPaddingInstruction<paddingInstructionStep1>);
            if constexpr (paddingInstructionStep1 == PaddingInstruction::Interrupt) {
                for (size_t i = 5; i < lengthStep1; ++i) {
                    *injectAtStep1++ = static_cast<char>(0xcc);
                }
            } else if constexpr (paddingInstructionStep1 == PaddingInstruction::Nop) {
                for (size_t i = 5; i < lengthStep1; ++i) {
                    *injectAtStep1++ = static_cast<char>(0x90);
                }
            }
        }
    }
    rv.JumpBackAddress = injectAtStep1;
    return rv;
}
} // namespace SenPatcher::x64

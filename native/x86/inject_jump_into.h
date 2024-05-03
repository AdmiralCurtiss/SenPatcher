#pragma once

#include <array>
#include <bit>
#include <cstring>

#include "util/logger.h"
#include "x86/emitter.h"
#include "x86/page_unprotect.h"

namespace SenPatcher::x86 {
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
static InjectJumpIntoCodeResult<length>
    InjectJumpIntoCode(HyoutaUtils::Logger& logger, char* injectAt, char* jumpTarget) {
    static_assert(length >= 5);

    InjectJumpIntoCodeResult<length> rv;
    char* inject = injectAt;
    {
        PageUnprotect page(logger, inject, rv.OverwrittenInstructions.size());
        std::memcpy(rv.OverwrittenInstructions.data(), inject, rv.OverwrittenInstructions.size());

        BranchHelper4Byte branch;
        branch.SetTarget(jumpTarget);
        branch.WriteJump(inject, JumpCondition::JMP);
        if constexpr (length > 5) {
            static_assert(IsValidPaddingInstruction<paddingInstruction>);
            if constexpr (paddingInstruction == PaddingInstruction::Interrupt) {
                for (size_t i = 5; i < length; ++i) {
                    *inject++ = static_cast<char>(0xcc);
                }
            } else if constexpr (paddingInstruction == PaddingInstruction::Nop) {
                for (size_t i = 5; i < length; ++i) {
                    *inject++ = static_cast<char>(0x90);
                }
            }
        }
    }
    rv.JumpBackAddress = inject;
    return rv;
}
} // namespace SenPatcher::x86

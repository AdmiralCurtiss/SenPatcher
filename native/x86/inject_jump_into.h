#pragma once

#include <array>
#include <bit>
#include <cstring>

#include "logger.h"
#include "x86/emitter.h"
#include "x86/page_unprotect.h"

namespace SenPatcher::x86 {
template<size_t length>
struct InjectJumpIntoCodeResult {
    char* JumpBackAddress;
    std::array<char, length> OverwrittenInstructions;
};

template<size_t length>
static InjectJumpIntoCodeResult<length>
    InjectJumpIntoCode(SenPatcher::Logger& logger, char* injectAt, char* jumpTarget) {
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
            for (size_t i = 5; i < length; ++i) {
                *inject++ = static_cast<char>(0xcc);
            }
        }
    }
    rv.JumpBackAddress = inject;
    return rv;
}
} // namespace SenPatcher::x86

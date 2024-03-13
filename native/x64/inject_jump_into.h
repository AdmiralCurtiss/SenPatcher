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

template<size_t length>
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
            for (size_t i = 12; i < length; ++i) {
                *inject++ = 0xcc;
            }
        }
    }
    rv.JumpBackAddress = inject;
    return rv;
}
} // namespace SenPatcher::x64

#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void SwapBrokenMasterQuartzValuesForDisplay(HyoutaUtils::Logger& logger,
                                            char* textRegion,
                                            GameVersion version,
                                            char*& codespace,
                                            char* codespaceEnd) {
    using namespace SenPatcher::x64;
    if (version != GameVersion::English) {
        return; // bug only exists in the english version
    }

    // on function entry:
    // r8w -> 0, 1, or 2 depending on which part of the description to print
    // dx  -> master quartz ID
    // we need to swap when:
    // description is 1 (applies to all of these) and
    // ID is any of 0x0c85, 0x0c86, 0x0c8a, 0x0c8c, 0x0c91, 0x0c95, 0x0c98
    // additionally, on french text only when ID is 0x0c81
    // then swap xmm3 and xmm1 for the call to snprintf_s()

    // ...actually this kinda sucks to hardcode, so instead...
    // we insert a sentinel character at the start of the actual text description in the t_mstqrt
    // file. before the sprintf call, we check if this sentinel character is there. if yes we swap
    // xmm3 and xmm1, and advance string ptr by 1

    // stack from rsp+31h to rsp+37h looks unused, so stash our sentinel check flag in there...
    // rsp+34h looks good

    char* initSentinelCheckPos = textRegion + ((0x14027fd47u - 0x140001000u));
    constexpr size_t initSentinelCheckLen = 13;
    char* checkSentinelPos = textRegion + ((0x14028015bu - 0x140001000u));
    constexpr size_t checkSentinelLen = 13;
    char* fixParametersPos = textRegion + ((0x14028020eu - 0x140001000u));
    constexpr size_t fixParametersLen = 12;

    // first initialize sentinel check flag on stack near start of function
    {
        auto injectResult = InjectJumpIntoCode<initSentinelCheckLen>(
            logger, initSentinelCheckPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        WriteInstruction40(codespace, 0xc644243400); // mov byte ptr [rsp+34h],0

        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    // on strncat() call that appends the description string, check for the sentinel;
    // if it's there skip it and set the flag on the stack
    {
        // we're a bit constrained for space here so this is somewhat creative
        auto injectResult =
            InjectJumpIntoCode<checkSentinelLen>(logger, checkSentinelPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        // there's a pending jz, handle that
        BranchHelper1Byte skip_strncpy;
        skip_strncpy.WriteJump(codespace, JumpCondition::JZ);

        // rdx contains the address of the string, r8d is free to use
        BranchHelper1Byte return_to_function_short;
        WriteInstruction24(codespace, 0x448a02);   // mov r8b,byte ptr[rdx]
        WriteInstruction32(codespace, 0x4180f824); // cmp r8b,24h
        return_to_function_short.WriteJump(codespace,
                                           JumpCondition::JNE); // jne return_to_function_short
        WriteInstruction40(codespace, 0xc644243401);            // mov byte ptr [rsp+34h],1
        WriteInstruction24(codespace, 0x48ffc2);                // inc rdx
        return_to_function_short.SetTarget(codespace);

        // don't copy back the jz at the start of the overwritten instructions
        std::memcpy(
            codespace, overwrittenInstructions.data() + 2, overwrittenInstructions.size() - 2);
        codespace += (overwrittenInstructions.size() - 2);
        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);

        // skip the call strncpy
        skip_strncpy.SetTarget(codespace);
        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress + 6));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    // check sentinel flag and fix up parameters if it's set
    {
        auto injectResult =
            InjectJumpIntoCode<fixParametersLen>(logger, fixParametersPos, R64::RAX, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        BranchHelper1Byte exit_without_swap;
        WriteInstruction40(codespace, 0x448a4c2434);               // mov r9b,byte ptr[rsp+34h]
        WriteInstruction24(codespace, 0x4584c9);                   // test r9b,r9b
        exit_without_swap.WriteJump(codespace, JumpCondition::JZ); // jz exit_without_swap
        WriteInstruction32(codespace, 0xf30f7ed3);                 // movq xmm2,xmm3
        WriteInstruction32(codespace, 0xf30f7ed9);                 // movq xmm3,xmm1
        WriteInstruction32(codespace, 0xf30f7eca);                 // movq xmm1,xmm2
        exit_without_swap.SetTarget(codespace);

        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);
    }
}
} // namespace SenLib::Sen3

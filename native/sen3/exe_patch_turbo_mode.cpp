#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "util/memread.h"
#include "util/memwrite.h"

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen3 {
static char TurboActive = 0;
static char TurboButtonPressedLastFrame = 0;
static float RealTimeStep = 0.0f;

void PatchTurboMode(PatchExecData& execData, bool makeToggle, bool adjustTimersForTurbo) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x64;
    using JC = JumpCondition;

    char* const addressBeforeTurboEnabledCheck =
        GetCodeAddressJpEn(version, textRegion, 0x14012f7f2, 0x140132f82);
    char* const addressJumpAfterTestCheckButtonPressed =
        GetCodeAddressJpEn(version, textRegion, 0x14012f82f, 0x140132fbf);
    char* const addressLoadTimeStepForLipflaps12 =
        GetCodeAddressJpEn(version, textRegion, 0x1402c86d1, 0x1402cff31);
    char* const addressLoadTimeStepForLipflaps3 =
        GetCodeAddressJpEn(version, textRegion, 0x1402c8729, 0x1402cff89);
    char* const addressLoadTimeStepForLipflaps3Trampoline =
        GetCodeAddressJpEn(version, textRegion, 0x1402c84f4, 0x1402cfd54);
    char* const addressLoadTimeStepForActiveVoice =
        GetCodeAddressJpEn(version, textRegion, 0x1402c8ecb, 0x1402d072b);
    char* const addressLoadTimeStepForGameplayTimer =
        GetCodeAddressJpEn(version, textRegion, 0x1403210bc, 0x14032a5cc);

    // hook before the turbo-enabled-in-config check and store the unscaled timestamp.
    // we must do this before the check, otherwise our RealTimeStep is never written in the
    // turbo-disabled case
    {
        char* codespace = execData.Codespace;

        char* const clampValuePtr = codespace;
        HyoutaUtils::MemWrite::WriteAdvUInt32(codespace, 0x3d888889); // 1.0f / 15.0f

        const auto inject =
            InjectJumpIntoCode<12>(logger, addressBeforeTurboEnabledCheck, R64::RDI, codespace);
        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        // store the unscaled timestep so we can use it later for the functions that want that.
        // clamp to 1.0f / 15.0f so long pauses don't cause havoc.
        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(&RealTimeStep));
        WriteInstruction24(codespace, 0x0f28c1);   // movaps xmm0,xmm1
        WriteInstruction32(codespace, 0xf30f5d05); // minss xmm0,dword ptr[clampValuePtr]
        int32_t clampValuePtrRelative = static_cast<int32_t>(clampValuePtr - (codespace + 4));
        std::memcpy(codespace, &clampValuePtrRelative, 4);
        codespace += 4;
        WriteInstruction32(codespace, 0xf30f1101); // movss dword ptr[rcx],xmm0

        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);

        execData.Codespace = codespace;
    }

    // hook the function that checks for turbo button held
    {
        char* codespace = execData.Codespace;

        const auto inject = InjectJumpIntoCode<12>(
            logger, addressJumpAfterTestCheckButtonPressed, R64::RCX, codespace);
        const int32_t relativeOffsetForJump =
            static_cast<int32_t>(static_cast<int8_t>(inject.OverwrittenInstructions[1]));
        char* const absolutePositionForJump =
            addressJumpAfterTestCheckButtonPressed + 2 + relativeOffsetForJump;

        // al is now nonzero if the turbo button is held, or zero if it's not
        // we need to return in al whether turbo should be active for the next frame

        BranchHelper1Byte jmpBackShort;
        if (makeToggle) {
            Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(&TurboActive));
            Emit_MOV_R64_IMM64(
                codespace, R64::RDX, std::bit_cast<uint64_t>(&TurboButtonPressedLastFrame));

            // in the toggle case, we need to check the button press state this frame and last frame
            // and flip the flag if last frame is zero and current frame is nonzero
            WriteInstruction16(codespace, 0x84c0); // test al,al

            BranchHelper1Byte toggleTestDone;
            toggleTestDone.WriteJump(codespace, JC::JZ);

            // currently pressed, was it pressed last frame too?
            WriteInstruction16(codespace, 0x8402); // test byte ptr[rdx],al
            toggleTestDone.WriteJump(codespace, JC::JNZ);

            // not pressed last frame
            WriteInstruction16(codespace, 0x8802); // mov byte ptr[rdx],al

            // flip the flag and move into return register
            Emit_MOVZX_R32_BytePtrR64(codespace, R32::EAX, R64::RCX);
            Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
            BranchHelper1Byte setTo1;
            setTo1.WriteJump(codespace, JC::JZ);
            BranchHelper1Byte flipDone;
            Emit_MOV_R64_IMM64(codespace, R64::RAX, 0);
            flipDone.WriteJump(codespace, JC::JMP);
            setTo1.SetTarget(codespace);
            Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);
            flipDone.SetTarget(codespace);

            // store to memory so we can keep track of whether turbo is active or not
            WriteInstruction16(codespace, 0x8801); // mov byte ptr[rcx],al
            jmpBackShort.WriteJump(codespace, JC::JMP);

            toggleTestDone.SetTarget(codespace);
            WriteInstruction16(codespace, 0x8802); // mov byte ptr[rdx],al
            Emit_MOVZX_R32_BytePtrR64(codespace, R32::EAX, R64::RCX);
        } else {
            // in the non-toggle case, we can just leave the value alone, it's already correct
            // just write it to memory so we can keep track of the current state
            Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(&TurboActive));
            WriteInstruction16(codespace, 0x8801); // mov byte ptr[rcx],al
        }

        BranchHelper1Byte turboNotActive;
        jmpBackShort.SetTarget(codespace);

        WriteInstruction16(codespace, 0x84c0); // test al,al
        turboNotActive.WriteJump(codespace, JC::JZ);

        // turbo active
        std::memcpy(codespace,
                    &inject.OverwrittenInstructions[2],
                    inject.OverwrittenInstructions.size() - 2);
        codespace += (inject.OverwrittenInstructions.size() - 2);
        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);

        // turbo not active
        turboNotActive.SetTarget(codespace);
        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(absolutePositionForJump));
        Emit_JMP_R64(codespace, R64::RCX);

        execData.Codespace = codespace;
    }

    // use the unscaled timestep for lipflaps
    // this is a multi-step process as the timestep is transformed twice before being passed to the
    // function that actually applies the time advancement
    // luckily, xmm13 is used by this function but only *after* we're done with the value, so we can
    // use that as our storage and don't have to write to memory between the steps!
    if (adjustTimersForTurbo) {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode<13, PaddingInstruction::Nop>(
            logger, addressLoadTimeStepForLipflaps12, R64::RCX, codespace);

        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(&RealTimeStep));
        WriteInstruction40(codespace, 0xf3440f1029); // movss xmm13,dword ptr[rcx]
        WriteInstruction40(codespace, 0xf3440f59ee); // mulss xmm13,xmm6

        // mov rcx,rbx; mulss xmm6,xmm9
        std::memcpy(codespace, inject.OverwrittenInstructions.data(), 8);
        codespace += 8;

        // we skip a call instruction here, replicate the result of that
        WriteInstruction64(codespace, 0xf30f1081d83c0000); // movss xmm0,dword ptr[rcx+0x3cd8]

        WriteInstruction40(codespace, 0xf3440f59e8); // mulss xmm13,xmm0

        Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);

        execData.Codespace = codespace;
    }
    if (adjustTimersForTurbo) {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode2Step<10, 12, PaddingInstruction::Nop>(
            logger,
            addressLoadTimeStepForLipflaps3,
            addressLoadTimeStepForLipflaps3Trampoline,
            R64::RDX,
            codespace);

        WriteInstruction32(codespace, 0x410f28cd); // movaps xmm1,xmm13

        std::memcpy(codespace, inject.OverwrittenInstructions.data(), 7);
        codespace += 7;

        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RDX);

        execData.Codespace = codespace;
    }

    // use unscaled timestep for active voices
    if (adjustTimersForTurbo) {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode<18, PaddingInstruction::Nop>(
            logger, addressLoadTimeStepForActiveVoice, R64::RAX, codespace);

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(&RealTimeStep));
        WriteInstruction32(codespace, 0xf30f1008); // movss xmm1,dword ptr[rax]

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);

        execData.Codespace = codespace;
    }

    // use unscaled timestep for gameplay timer
    if (adjustTimersForTurbo) {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode<12>(
            logger, addressLoadTimeStepForGameplayTimer, R64::RAX, codespace);

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(&RealTimeStep));
        WriteInstruction32(codespace, 0xf30f1000); // movss xmm0,dword ptr[rax]
        WriteInstruction32(codespace, 0xf30f5ac0); // cvtss2sd xmm0,xmm0

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RAX);

        execData.Codespace = codespace;
    }
}
} // namespace SenLib::Sen3

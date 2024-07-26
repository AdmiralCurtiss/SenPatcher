#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "util/memread.h"

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen3 {
static char TurboActive = 0;
static char TurboButtonPressedLastFrame = 0;

void PatchTurboMode(PatchExecData& execData, bool makeToggle) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x64;
    using JC = JumpCondition;

    char* const addressJumpAfterTestCheckButtonPressed =
        GetCodeAddressJpEn(version, textRegion, 0x14012f82f, 0x140132fbf);

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
}
} // namespace SenLib::Sen3

#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "util/memread.h"

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen1 {
static char TurboActive = 0;
static char TurboButtonPressedLastFrame = 0;

void PatchTurboMode(PatchExecData& execData,
                    bool removeAutoSkip,
                    int turboModeButton,
                    bool enableR2,
                    bool makeToggle) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;
    using JC = JumpCondition;

    char* const addressButtonTurboMode =
        GetCodeAddressJpEn(version, textRegion, 0x48179a, 0x48300a);
    char* const addressCallCheckButtonPressed =
        GetCodeAddressJpEn(version, textRegion, 0x48179d, 0x48300d);
    char* const addressJumpR2NotebookOpen =
        GetCodeAddressJpEn(version, textRegion, 0x5b6fbf, 0x5b812f);
    char* const addressJumpR2NotebookSettings =
        GetCodeAddressJpEn(version, textRegion, 0x6de1a0, 0x6dfaf0);
    char* const addressButtonBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4d6404, 0x4d7c54);
    const size_t lengthButtonBattleAnimationAutoSkip = 2 + 2 + 2 + 5;
    char* const addressButtonBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4f0a22, 0x4f2242);
    const size_t lengthButtonBattleResultsAutoSkip = 2 + 2 + 2 + 5;
    char* const addressJumpBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4d6411, 0x4d7c61);
    char* const addressJumpBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4f0a2f, 0x4f224f);

    // replace the button that is checked for turbo, if it is valid
    if (turboModeButton < 0 || turboModeButton > 0xF) {
        logger.Log("Invalid turbo mode button, skipping patch.\n");
    } else {
        const char button = static_cast<char>(turboModeButton);
        PageUnprotect page(logger, addressButtonTurboMode, 1);
        *addressButtonTurboMode = button;
    }

    // hook the function that checks for turbo button held
    {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode<5>(logger, addressCallCheckButtonPressed, codespace);
        const int32_t relativeOffsetForCall = static_cast<int32_t>(
            HyoutaUtils::MemRead::ReadUInt32(&inject.OverwrittenInstructions[1]));
        char* const absolutePositionForCall = inject.JumpBackAddress + relativeOffsetForCall;
        BranchHelper4Byte call;
        call.SetTarget(absolutePositionForCall);
        call.WriteJump(codespace, JC::CALL);

        // al is now nonzero if the turbo button is held, or zero if it's not
        // we need to return in al whether turbo should be active for the next frame

        BranchHelper1Byte jmpBackShort;
        if (makeToggle) {
            // in the toggle case, we need to check the button press state this frame and last frame
            // and flip the flag if last frame is zero and current frame is nonzero
            Emit_TEST_R8_R8(codespace, R8::AL, R8::AL);

            BranchHelper1Byte toggleTestDone;
            toggleTestDone.WriteJump(codespace, JC::JZ);

            // currently pressed, was it pressed last frame too?
            Emit_TEST_R8_BytePtr(codespace, R8::AL, &TurboButtonPressedLastFrame);
            toggleTestDone.WriteJump(codespace, JC::JNZ);

            // not pressed last frame
            Emit_MOV_BytePtr_R8(codespace, &TurboButtonPressedLastFrame, R8::AL);

            // flip the flag and move into return register
            Emit_MOV_R8_BytePtr(codespace, R8::AL, &TurboActive);
            Emit_TEST_R8_R8(codespace, R8::AL, R8::AL);
            BranchHelper1Byte setTo1;
            setTo1.WriteJump(codespace, JC::JZ);
            BranchHelper1Byte flipDone;
            Emit_MOV_R8_IMM8(codespace, R8::AL, 0);
            flipDone.WriteJump(codespace, JC::JMP);
            setTo1.SetTarget(codespace);
            Emit_MOV_R8_IMM8(codespace, R8::AL, 1);
            flipDone.SetTarget(codespace);

            // store to memory so we can keep track of whether turbo is active or not
            Emit_MOV_BytePtr_R8(codespace, &TurboActive, R8::AL);
            jmpBackShort.WriteJump(codespace, JC::JMP);

            toggleTestDone.SetTarget(codespace);
            Emit_MOV_BytePtr_R8(codespace, &TurboButtonPressedLastFrame, R8::AL);
            Emit_MOV_R8_BytePtr(codespace, R8::AL, &TurboActive);
        } else {
            // in the non-toggle case, we can just leave the value alone, it's already correct
            // just write it to memory so we can keep track of the current state
            Emit_MOV_BytePtr_R8(codespace, &TurboActive, R8::AL);
        }

        BranchHelper4Byte jmpBack;
        jmpBackShort.SetTarget(codespace);
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }

    // replace the code that checks for a pressed button in turbo mode
    // with code that just checks the turbo mode flag
    {
        char* tmp = addressButtonBattleAnimationAutoSkip;
        size_t len = lengthButtonBattleAnimationAutoSkip;
        char* end = tmp + len;
        PageUnprotect page(logger, tmp, len);
        Emit_MOV_R8_BytePtr(tmp, R8::AL, &TurboActive);
        while (tmp < end) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
    {
        char* tmp = addressButtonBattleResultsAutoSkip;
        size_t len = lengthButtonBattleResultsAutoSkip;
        char* end = tmp + len;
        PageUnprotect page(logger, tmp, len);
        Emit_MOV_R8_BytePtr(tmp, R8::AL, &TurboActive);
        while (tmp < end) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }


    if (enableR2) {
        {
            char* tmp = addressJumpR2NotebookOpen;
            PageUnprotect page(logger, tmp, 2);
            for (size_t i = 0; i < 2; ++i) {
                *tmp++ = static_cast<char>(0x90); // nop
            }
        }
        {
            char* tmp = addressJumpR2NotebookSettings;
            PageUnprotect page(logger, tmp, 6);
            for (size_t i = 0; i < 6; ++i) {
                *tmp++ = static_cast<char>(0x90); // nop
            }
        }
    }

    if (removeAutoSkip) {
        {
            char* tmp = addressJumpBattleAnimationAutoSkip;
            PageUnprotect page(logger, tmp, 1);
            *tmp++ = static_cast<char>(0xeb); // jmp
        }
        {
            char* tmp = addressJumpBattleResultsAutoSkip;
            PageUnprotect page(logger, tmp, 2);
            *tmp++ = static_cast<char>(0x90); // nop
            *tmp++ = static_cast<char>(0xe9); // jmp
        }
    }
}
} // namespace SenLib::Sen1

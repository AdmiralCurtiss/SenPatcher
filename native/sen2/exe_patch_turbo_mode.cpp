#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "util/memread.h"

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen2 {
static char TurboActive = 0;
static char TurboButtonPressedLastFrame = 0;

void PatchTurboMode(PatchExecData& execData, bool removeAutoSkip, bool makeToggle) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;
    using JC = JumpCondition;

    using namespace SenPatcher::x86;
    char* const addressCallCheckButtonPressed =
        GetCodeAddressJpEn(version, textRegion, 0x6af507, 0x6b0557);
    char* const addressButtonBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x479b0e, 0x479c7e);
    const size_t lengthButtonBattleAnimationAutoSkip = 2 + 2 + 2 + 5;
    char* const addressButtonBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4929ad, 0x492bdd);
    const size_t lengthButtonBattleResultsAutoSkip = 2 + 2 + 2 + 5;
    char* const addressButtonBattleSomethingAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4845a1, 0x484741);
    const size_t lengthButtonBattleSomethingAutoSkip = 2 + 2 + 2 + 5;
    char* const addressButtonBattleStartAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4836de, 0x48387e);
    const size_t lengthButtonBattleStartAutoSkip = 2 + 2 + 2 + 5;
    char* const addressJumpBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x479b1b, 0x479c8b);
    char* const addressJumpBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4929ba, 0x492bea);
    char* const addressJumpBattleSomethingAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4845ae, 0x48474e);
    char* const addressJumpBattleStartAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4836eb, 0x48388b);

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
    {
        char* tmp = addressButtonBattleSomethingAutoSkip;
        size_t len = lengthButtonBattleSomethingAutoSkip;
        char* end = tmp + len;
        PageUnprotect page(logger, tmp, len);
        Emit_MOV_R8_BytePtr(tmp, R8::AL, &TurboActive);
        while (tmp < end) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
    {
        char* tmp = addressButtonBattleStartAutoSkip;
        size_t len = lengthButtonBattleStartAutoSkip;
        char* end = tmp + len;
        PageUnprotect page(logger, tmp, len);
        Emit_MOV_R8_BytePtr(tmp, R8::AL, &TurboActive);
        while (tmp < end) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }

    if (removeAutoSkip) {
        {
            PageUnprotect page(logger, addressJumpBattleAnimationAutoSkip, 1);
            *addressJumpBattleAnimationAutoSkip = static_cast<char>(0xeb); // jmp
        }
        {
            PageUnprotect page(logger, addressJumpBattleStartAutoSkip, 1);
            *addressJumpBattleStartAutoSkip = static_cast<char>(0xeb); // jmp
        }
        {
            PageUnprotect page(logger, addressJumpBattleSomethingAutoSkip, 1);
            *addressJumpBattleSomethingAutoSkip = static_cast<char>(0xeb); // jmp
        }
        {
            PageUnprotect page(logger, addressJumpBattleResultsAutoSkip, 2);
            *addressJumpBattleResultsAutoSkip = static_cast<char>(0x90);       // nop
            *(addressJumpBattleResultsAutoSkip + 1) = static_cast<char>(0xe9); // jmp
        }
    }
}
} // namespace SenLib::Sen2
#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchDisablePauseOnFocusLoss(SenPatcher::Logger& logger,
                                  char* textRegion,
                                  GameVersion version,
                                  char*& codespace,
                                  char* codespaceEnd) {
    using namespace SenPatcher::x86;

    // 0x413660 -> this branch is false when game is inactive, which skips most of the game update
    //             logic
    // 0x413A10 -> GameHasFocusCached() or something along those lines?
    // 0x412520 -> corresponding setter to the above
    // 0x4161BE -> called from here
    // 0x415cc0 -> function registered as the window procedure in SetWindowLongA()

    char* silenceAudioIfUnfocusedPos1 = GetCodeAddressJpEn(version, textRegion, 0x4161b1, 0x4161c1);
    char* silenceAudioIfUnfocusedPos2 = GetCodeAddressJpEn(version, textRegion, 0x4161f9, 0x416209);
    char* runMainGameLoopIfUnfocusedPos =
        GetCodeAddressJpEn(version, textRegion, 0x413650, 0x413660);
    char* skipMouseButtonsIfUnfocusedBackPos =
        GetCodeAddressJpEn(version, textRegion, 0x6ad622, 0x6ae652);
    char* skipMouseButtonsIfUnfocusedMouseButtonPos =
        GetCodeAddressJpEn(version, textRegion, 0x6ad61b, 0x6ae64b);
    char* gameStateAddress = GetCodeAddressJpEn(version, textRegion, 0x116f3c0, 0x1173c40);
    char* getKeyStateAddress;
    std::memcpy(
        &getKeyStateAddress, GetCodeAddressJpEn(version, textRegion, 0x6ad61e, 0x6ae64e), 4);

    // don't silence audio output when unfocused
    {
        PageUnprotect page(logger, silenceAudioIfUnfocusedPos1, 2);
        char* tmp = silenceAudioIfUnfocusedPos1;
        BranchHelper1Byte branch;
        branch.SetTarget(silenceAudioIfUnfocusedPos2);
        branch.WriteJump(tmp, JumpCondition::JMP);
    }

    // still run main game loop when unfocused
    {
        char* tmp = runMainGameLoopIfUnfocusedPos;
        PageUnprotect page(logger, tmp, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }

    // avoid processing mouse clicks when unfocused
    // (this previously happened only implicitly because the game didn't run...)
    // assemble some logic to skip mouse button processing when unfocused

    {
        PageUnprotect page(logger, skipMouseButtonsIfUnfocusedMouseButtonPos, 5 * 0x11);

        BranchHelper4Byte jump_to_codespace;
        BranchHelper4Byte back_to_function;
        BranchHelper1Byte skip_processing;
        back_to_function.SetTarget(skipMouseButtonsIfUnfocusedBackPos);

        // CS2 has this copy-pasted 5 times for the 5 supported mouse buttons so inject in all of
        // them...
        for (int i = 0; i < 5; ++i) {
            char* tmp = (skipMouseButtonsIfUnfocusedMouseButtonPos + i * 0x11);
            jump_to_codespace.WriteJump(tmp, JumpCondition::JMP); // jmp jump_to_codespace
            *tmp++ = static_cast<char>(0x90);                     // nop
            *tmp++ = static_cast<char>(0x90);                     // nop
        }

        jump_to_codespace.SetTarget(codespace);

        WriteInstruction16(codespace, 0x8b0d); // mov ecx,[static_address_that_holds_game_state]
        std::memcpy(codespace, &gameStateAddress, 4);
        codespace += 4;
        WriteInstruction16(codespace, 0x85c9);                   // test ecx,ecx
        skip_processing.WriteJump(codespace, JumpCondition::JZ); // jz skip_processing
        WriteInstruction48(codespace, 0x8a89c4090000);           // mov cl,byte ptr[ecx+9c4h]

        // cl now holds 0 if unfocused, 1 if focused

        WriteInstruction16(codespace, 0x84c9);                   // test cl,cl
        skip_processing.WriteJump(codespace, JumpCondition::JZ); // jz skip_processing
        WriteInstruction16(codespace, 0x8b0d);                   // mov ecx,[USER32.DLL:GetKeyState]
        std::memcpy(codespace, &getKeyStateAddress, 4);
        codespace += 4;
        WriteInstruction16(codespace, 0x85c9);                     // test ecx,ecx
        skip_processing.WriteJump(codespace, JumpCondition::JZ);   // jz skip_processing
        *codespace++ = static_cast<char>(0x50);                    // push eax
        WriteInstruction16(codespace, 0xffd1);                     // call ecx
        back_to_function.WriteJump(codespace, JumpCondition::JMP); // jmp back_to_function
        skip_processing.SetTarget(codespace);
        WriteInstruction16(codespace, 0x33c0);                     // xor eax,eax
        back_to_function.WriteJump(codespace, JumpCondition::JMP); // jmp back_to_function
    }
}
} // namespace SenLib::Sen2

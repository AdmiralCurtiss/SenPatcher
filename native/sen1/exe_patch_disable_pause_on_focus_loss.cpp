#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchDisablePauseOnFocusLoss(HyoutaUtils::Logger& logger,
                                  char* textRegion,
                                  GameVersion version,
                                  char*& codespace,
                                  char* codespaceEnd) {
    using namespace SenPatcher::x86;

    // 0x444AA0 -> game active setter
    // 0x444AF0 -> game active getter

    char* silenceAudioIfUnfocusedPos1 = GetCodeAddressJpEn(version, textRegion, 0x447d6a, 0x447eba);
    char* silenceAudioIfUnfocusedPos2 = GetCodeAddressJpEn(version, textRegion, 0x447da6, 0x447ef6);
    char* runMainGameLoopIfUnfocusedPos =
        GetCodeAddressJpEn(version, textRegion, 0x441c00, 0x441d50);
    char* skipMouseButtonsIfUnfocusedBackPos =
        GetCodeAddressJpEn(version, textRegion, 0x479ad8, 0x47b348);
    char* skipMouseButtonsIfUnfocusedMouseButtonPos =
        GetCodeAddressJpEn(version, textRegion, 0x479ad1, 0x47b341);
    char* gameStateAddress = GetCodeAddressJpEn(version, textRegion, 0x1361c28, 0x1363fc8);
    char* getKeyStateAddress;
    std::memcpy(
        &getKeyStateAddress, GetCodeAddressJpEn(version, textRegion, 0x479ad4, 0x47b344), 4);

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
        BranchHelper4Byte jump_to_codespace;
        BranchHelper4Byte back_to_function;
        BranchHelper1Byte skip_processing;
        jump_to_codespace.SetTarget(codespace);
        back_to_function.SetTarget(skipMouseButtonsIfUnfocusedBackPos);

        {
            PageUnprotect page(logger, skipMouseButtonsIfUnfocusedMouseButtonPos, 7);
            char* tmp = skipMouseButtonsIfUnfocusedMouseButtonPos;
            jump_to_codespace.WriteJump(tmp, JumpCondition::JMP); // jmp jump_to_codespace
            *tmp++ = static_cast<char>(0x90);                     // nop
            *tmp++ = static_cast<char>(0x90);                     // nop
        }

        WriteInstruction16(codespace, 0x8b0d); // mov ecx,[static_address_that_holds_game_state]
        std::memcpy(codespace, &gameStateAddress, 4);
        codespace += 4;
        WriteInstruction16(codespace, 0x85c9);                   // test ecx,ecx
        skip_processing.WriteJump(codespace, JumpCondition::JZ); // jz skip_processing
        WriteInstruction48(codespace, 0x8a89b8070000);           // mov cl,byte ptr[ecx+7b8h]

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
} // namespace SenLib::Sen1

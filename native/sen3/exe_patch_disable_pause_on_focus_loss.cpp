#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDisablePauseOnFocusLoss(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* silenceAudioIfUnfocusedPos1 = GetCodeAddressJpEn(version,
                                                           textRegion,
                                                           Addresses{.Jp106 = 0x1400f943b,
                                                                     .En106 = 0x1400f9abb,
                                                                     .Jp107 = 0x1400f943b,
                                                                     .En107 = 0x1400f9abb});
    char* silenceAudioIfUnfocusedPos2 = GetCodeAddressJpEn(version,
                                                           textRegion,
                                                           Addresses{.Jp106 = 0x1400f9474,
                                                                     .En106 = 0x1400f9af4,
                                                                     .Jp107 = 0x1400f9474,
                                                                     .En107 = 0x1400f9af4});
    char* runMainGameLoopIfUnfocusedPos = GetCodeAddressJpEn(version,
                                                             textRegion,
                                                             Addresses{.Jp106 = 0x1400f7024,
                                                                       .En106 = 0x1400f76a4,
                                                                       .Jp107 = 0x1400f7024,
                                                                       .En107 = 0x1400f76a4});
    char* skipMouseButtonsIfUnfocusedPos1 = GetCodeAddressJpEn(version,
                                                               textRegion,
                                                               Addresses{.Jp106 = 0x14012dfd2,
                                                                         .En106 = 0x140131552,
                                                                         .Jp107 = 0x14012dfd2,
                                                                         .En107 = 0x140131552});
    char* skipMouseButtonsIfUnfocusedPos2 = GetCodeAddressJpEn(version,
                                                               textRegion,
                                                               Addresses{.Jp106 = 0x141d43de0,
                                                                         .En106 = 0x141d59f80,
                                                                         .Jp107 = 0x141d43e00,
                                                                         .En107 = 0x141d59fa0});

    // don't silence audio output when unfocused
    size_t distance =
        static_cast<size_t>(silenceAudioIfUnfocusedPos2 - silenceAudioIfUnfocusedPos1);
    PageUnprotect page2(logger, silenceAudioIfUnfocusedPos1, distance);
    {
        char* tmp = silenceAudioIfUnfocusedPos1;
        BranchHelper1Byte branch;
        branch.SetTarget(silenceAudioIfUnfocusedPos2);
        branch.WriteJump(tmp, JumpCondition::JMP);

        // the rest of this we reserve for the mouse click asm below
        // (there's not enough space there to write a 12byte absolute jmp)
        for (size_t i = 2; i < distance; ++i) {
            *tmp++ = static_cast<char>(0xcc); // int 3
        }
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
        char* codespaceStart = (silenceAudioIfUnfocusedPos1) + 2;
        char* codespaceEnd = silenceAudioIfUnfocusedPos2;
        char* codespace = codespaceStart;

        BranchHelper4Byte jump_to_codespace;
        jump_to_codespace.SetTarget(codespace);
        BranchHelper4Byte back_to_function;
        BranchHelper1Byte back_to_function_short;
        BranchHelper1Byte skip_processing;

        back_to_function.SetTarget(skipMouseButtonsIfUnfocusedPos1 + 6);
        int32_t relativeGetKeyStateAddress;
        std::memcpy(&relativeGetKeyStateAddress, skipMouseButtonsIfUnfocusedPos1 + 2, 4);
        char* GetKeyStateAddress =
            relativeGetKeyStateAddress + (skipMouseButtonsIfUnfocusedPos1 + 6);
        char* GameStateAddress = skipMouseButtonsIfUnfocusedPos2;

        using JC = JumpCondition;
        {
            char* tmp = skipMouseButtonsIfUnfocusedPos1;
            PageUnprotect page(logger, tmp, 6);
            jump_to_codespace.WriteJump(tmp, JC::JMP); // jmp jump_to_codespace
            *tmp = static_cast<char>(0x90);            // nop
        }

        const auto WriteRelativeAddress32 = [](char*& codepos, char* absoluteTarget) {
            char* absoluteHere = codepos + 4;
            int32_t a = static_cast<int32_t>(absoluteTarget - absoluteHere);
            std::memcpy(codepos, &a, 4);
            codepos += 4;
        };

        WriteInstruction24(codespace, 0x488d05u); // lea rax,[address_that_holds_game_state]
        WriteRelativeAddress32(codespace, GameStateAddress);
        WriteInstruction24(codespace, 0x488b00u);     // mov rax,[rax]
        WriteInstruction24(codespace, 0x4885c0u);     // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        WriteInstruction24(codespace, 0x0fb680u);     // movzx eax,byte ptr[rax+1ce0h]
        WriteInstruction32(codespace, 0xe01c0000u);
        WriteInstruction16(codespace, 0x85c0);        // test eax,eax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        WriteInstruction24(codespace, 0x488d05u);     // lea rax,[GetKeyState]
        WriteRelativeAddress32(codespace, GetKeyStateAddress);
        WriteInstruction24(codespace, 0x488b00u);             // mov rax,[rax]
        WriteInstruction24(codespace, 0x4885c0u);             // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ);         // jz skip_processing
        WriteInstruction16(codespace, 0xffd0);                // call rax
        back_to_function_short.WriteJump(codespace, JC::JMP); // jmp back_to_function_short
        skip_processing.SetTarget(codespace);
        WriteInstruction24(codespace, 0x6633c0); // xor ax,ax
        back_to_function_short.SetTarget(codespace);
        back_to_function.WriteJump(codespace, JC::JMP); // jmp back_to_function
    }
}
} // namespace SenLib::Sen3

#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchDisablePauseOnFocusLoss(SenPatcher::Logger& logger,
                                  char* textRegion,
                                  GameVersion version,
                                  char*& codespace_,
                                  char* codespaceEnd_) {
    using namespace SenPatcher::x64;
    char* silenceAudioIfUnfocusedPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x1400b3461, 0x1400b3481);
    char* silenceAudioIfUnfocusedPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x1400b34a4, 0x1400b34c4);
    char* runMainGameLoopIfUnfocusedPos =
        GetCodeAddressJpEn(version, textRegion, 0x1400b12f4, 0x1400b1314);
    char* skipMouseButtonsIfUnfocusedPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x1400e9e4c, 0x1400ebf0c);
    char* skipMouseButtonsIfUnfocusedPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x143784650, 0x1437877d0);

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

    const auto write_instruction_32 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 24) & 0xff);
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_24 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 16) & 0xff);
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };
    const auto write_instruction_16 = [&](char*& codepos, uint32_t instr) {
        *codepos++ = (char)((instr >> 8) & 0xff);
        *codepos++ = (char)(instr & 0xff);
    };

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

        write_instruction_24(codespace, 0x488d05u); // lea rax,[address_that_holds_game_state]
        WriteRelativeAddress32(codespace, GameStateAddress);
        write_instruction_24(codespace, 0x488b00u);   // mov rax,[rax]
        write_instruction_24(codespace, 0x4885c0u);   // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        write_instruction_24(codespace, 0x0fb680u);   // movzx eax,byte ptr[rax+1cb0h]
        write_instruction_32(codespace, 0xb01c0000u);
        write_instruction_16(codespace, 0x85c0);      // test eax,eax
        skip_processing.WriteJump(codespace, JC::JZ); // jz skip_processing
        write_instruction_24(codespace, 0x488d05u);   // lea rax,[GetKeyState]
        WriteRelativeAddress32(codespace, GetKeyStateAddress);
        write_instruction_24(codespace, 0x488b00u);           // mov rax,[rax]
        write_instruction_24(codespace, 0x4885c0u);           // test rax,rax
        skip_processing.WriteJump(codespace, JC::JZ);         // jz skip_processing
        write_instruction_16(codespace, 0xffd0);              // call rax
        back_to_function_short.WriteJump(codespace, JC::JMP); // jmp back_to_function_short
        skip_processing.SetTarget(codespace);
        write_instruction_24(codespace, 0x6633c0); // xor ax,ax
        back_to_function_short.SetTarget(codespace);
        back_to_function.WriteJump(codespace, JC::JMP); // jmp back_to_function
    }
}
} // namespace SenLib::Sen4

#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchFixPcConfirmCancelWhenSwapped(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    // inject right after the 'id' value is read and stored from the XML
    char* injectpos = GetCodeAddressJpEn(version,
                                         textRegion,
                                         Addresses{.Jp121 = 0x1400e914e,
                                                   .En121 = 0x1400eb20e,
                                                   .Jp122 = 0x1400e914e,
                                                   .En122 = 0x1400eb20e});
    char* isSwitchButtonFuncPos = GetCodeAddressJpEn(version,
                                                     textRegion,
                                                     Addresses{.Jp121 = 0x1405fa7b0,
                                                               .En121 = 0x1405fcd30,
                                                               .Jp122 = 0x1405fa710,
                                                               .En122 = 0x1405fcc90});
    char* isDynamicPromptsFuncPos = GetCodeAddressJpEn(version,
                                                       textRegion,
                                                       Addresses{.Jp121 = 0x1405fa810,
                                                                 .En121 = 0x1405fcd90,
                                                                 .Jp122 = 0x1405fa770,
                                                                 .En122 = 0x1405fccf0});
    char* getPcSettingsFuncPos = GetCodeAddressJpEn(version,
                                                    textRegion,
                                                    Addresses{.Jp121 = 0x1405fa670,
                                                              .En121 = 0x1405fcbf0,
                                                              .Jp122 = 0x1405fa5d0,
                                                              .En122 = 0x1405fcb50});

    {
        // our chosen injection location has some relative addresses, so untangle those
        int32_t relativeAddressStringBinding;
        std::memcpy(&relativeAddressStringBinding, injectpos + 3, 4);
        char* absoluteAddressStringBinding = injectpos + 3 + 4 + relativeAddressStringBinding;
        int32_t relativeAddressReadXmlElem;
        std::memcpy(&relativeAddressReadXmlElem, injectpos + 11, 4);
        char* absoluteAddressReadXmlElem = injectpos + 11 + 4 + relativeAddressReadXmlElem;

        auto injectResult = InjectJumpIntoCode<15>(logger, injectpos, R64::RDX, codespace);

        BranchHelper1Byte jumpBackShort;
        BranchHelper1Byte check5;

        // check if both swapped buttons are enabled and dynamic prompts are disabled
        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(getPcSettingsFuncPos));
        Emit_CALL_R64(codespace, R64::RDX);              // call getPcSettings
        Emit_MOV_R64_R64(codespace, R64::RCX, R64::RAX); // mov rcx,rax
        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(isSwitchButtonFuncPos));
        Emit_CALL_R64(codespace, R64::RDX);                    // call isSwitchButton
        WriteInstruction16(codespace, 0x84c0);                 // test al,al
        jumpBackShort.WriteJump(codespace, JumpCondition::JZ); // jz jumpBackShort
        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(getPcSettingsFuncPos));
        Emit_CALL_R64(codespace, R64::RDX);              // call getPcSettings
        Emit_MOV_R64_R64(codespace, R64::RCX, R64::RAX); // mov rcx,rax
        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(isDynamicPromptsFuncPos));
        Emit_CALL_R64(codespace, R64::RDX);                     // call isDynamicPrompts
        WriteInstruction16(codespace, 0x84c0);                  // test al,al
        jumpBackShort.WriteJump(codespace, JumpCondition::JNZ); // jnz jumpBackShort

        // checks out, so swap around config for 4 and 5
        WriteInstruction40(codespace, 0x488d442438);            // lea rax,[rsp+38h]
        WriteInstruction16(codespace, 0x8b08);                  // mov ecx,dword ptr[rax]
        WriteInstruction24(codespace, 0x83f904);                // cmp ecx,4
        check5.WriteJump(codespace, JumpCondition::JNE);        // jne check5
        WriteInstruction48(codespace, 0xc70005000000);          // mov dword ptr[rax],5
        jumpBackShort.WriteJump(codespace, JumpCondition::JMP); // jmp jumpBackShort
        check5.SetTarget(codespace);
        WriteInstruction24(codespace, 0x83f905);                // cmp ecx,5
        jumpBackShort.WriteJump(codespace, JumpCondition::JNE); // jne jumpBackShort
        WriteInstruction48(codespace, 0xc70004000000);          // mov dword ptr[rax],4

        jumpBackShort.SetTarget(codespace);

        // replace the code we overwrote with equivalent
        Emit_MOV_R64_IMM64(
            codespace, R64::RDX, std::bit_cast<uint64_t>(absoluteAddressStringBinding));
        Emit_MOV_R64_R64(codespace, R64::RCX, R64::R15);
        Emit_MOV_R64_IMM64(
            codespace, R64::RAX, std::bit_cast<uint64_t>(absoluteAddressReadXmlElem));
        Emit_CALL_R64(codespace, R64::RAX);

        // go back to inject location
        Emit_MOV_R64_IMM64(
            codespace, R64::RDX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RDX);
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen4

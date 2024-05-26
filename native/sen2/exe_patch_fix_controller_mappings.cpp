#include "exe_patch.h"

#include <algorithm>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchFixControllerMappings(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;
    char* const addressStructMemAlloc =
        GetCodeAddressJpEn(version, textRegion, 0x6ac97c, 0x6ad9ac) + 1;
    char* const addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x6ac02f, 0x6acf8f);
    char* const addressMapLookupCode = GetCodeAddressJpEn(version, textRegion, 0x6ad70b, 0x6ae73b);
    constexpr size_t lengthMapLookupCodeForDelete = 0x36;
    char* const addressMapLookupPops = addressMapLookupCode + (0x6ae76d - 0x6ae73b);
    char* const addressMapLookupSuccessPush = addressMapLookupCode + (0x6ae77a - 0x6ae73b);
    char* const addressMapLookupFailMov = addressMapLookupCode + (0x6ae771 - 0x6ae73b);

    // clang-format off

    // increase struct allocation by 0x20 bytes
    uint8_t allocLengthOld;
    {
        PageUnprotect page(logger, addressStructMemAlloc, 1);
        allocLengthOld = static_cast<uint8_t>(*addressStructMemAlloc);
        char allocLengthNew = static_cast<char>(allocLengthOld + 0x20);
        *addressStructMemAlloc = allocLengthNew;
    }

    using JC = JumpCondition;

    {
        BranchHelper4Byte jumpToNewCode;
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte begin_loop_1;
        BranchHelper1Byte begin_loop_2;
        BranchHelper1Byte lookup_1;
        BranchHelper1Byte lookup_2;
        BranchHelper1Byte lookup_3;
        BranchHelper1Byte lookup_4;
        BranchHelper1Byte lookup_5;
        BranchHelper1Byte lookup_6;
        BranchHelper1Byte lookup_fail;

        char* tmp = codespace;
        const auto injectResult = InjectJumpIntoCode<5>(logger, addressInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // initialize the lookup table so every button points at itself
        WriteInstruction24(tmp, 0x8b75c8);       // mov esi,dword ptr[ebp-38h]
        WriteInstruction24(tmp, 0x8d7e04);       // lea edi,[esi+4h]
        WriteInstruction16(tmp, 0x33c0);         // xor eax,eax
        begin_loop_1.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8d0c07);       // lea ecx,[edi+eax]
        WriteInstruction16(tmp, 0x8801);         // mov byte ptr[ecx],al
        WriteInstruction8(tmp, 0x40);            // inc eax
        WriteInstruction24(tmp, 0x83f820);       // cmp eax,20h
        begin_loop_1.WriteJump(tmp, JC::JB);     // jb begin_loop_1

        // look up each key in the (presumably) std::map<int, int>
        // and write it into the lookup table in the other direction
        WriteInstruction16(tmp, 0x33c0);         // xor eax,eax
        WriteInstruction16(tmp, 0x8b36);         // mov esi,dword ptr[esi]
        WriteInstruction24(tmp, 0x8b7608);       // mov esi,dword ptr[esi+8h]
        begin_loop_2.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8b4e04);       // mov ecx,dword ptr[esi+04h]
        WriteInstruction32(tmp, 0x80790d00);     // cmp byte ptr[ecx+0dh],0
        WriteInstruction16(tmp, 0x8bd6);         // mov edx,esi
        lookup_4.WriteJump(tmp, JC::JNZ);        // jnz lookup_4
        lookup_1.SetTarget(tmp);
        WriteInstruction24(tmp, 0x394110);       // cmp dword ptr[ecx+10h],eax
        lookup_2.WriteJump(tmp, JC::JGE);        // jge lookup_2
        WriteInstruction24(tmp, 0x8b4908);       // mov ecx,dword ptr[ecx+08h]
        lookup_3.WriteJump(tmp, JC::JMP);        // jmp lookup_3
        lookup_2.SetTarget(tmp);
        WriteInstruction16(tmp, 0x8bd1);         // mov edx,ecx
        WriteInstruction16(tmp, 0x8b09);         // mov ecx,dword ptr[ecx]
        lookup_3.SetTarget(tmp);
        WriteInstruction32(tmp, 0x80790d00);     // cmp byte ptr[ecx+0dh],0
        lookup_1.WriteJump(tmp, JC::JZ);         // jz  lookup_1
        lookup_4.SetTarget(tmp);
        WriteInstruction16(tmp, 0x3bd6);         // cmp edx,esi
        lookup_5.WriteJump(tmp, JC::JZ);         // jz  lookup_5
        WriteInstruction16(tmp, 0x8bca);         // mov ecx,edx
        WriteInstruction24(tmp, 0x3b4210);       // cmp eax,dword ptr[edx+10h]
        lookup_6.WriteJump(tmp, JC::JGE);        // jge lookup_6
        lookup_5.SetTarget(tmp);
        WriteInstruction16(tmp, 0x8bce);         // mov ecx,esi
        lookup_6.SetTarget(tmp);
        WriteInstruction16(tmp, 0x8bd1);         // mov edx,ecx
        WriteInstruction16(tmp, 0x3bd6);         // cmp edx,esi
        WriteInstruction16(tmp, 0x8bc8);         // mov ecx,eax
        lookup_fail.WriteJump(tmp, JC::JZ);      // jz  lookup_fail
        WriteInstruction24(tmp, 0x8b4a14);       // mov ecx,dword ptr[edx+14h]
        lookup_fail.SetTarget(tmp);
        WriteInstruction24(tmp, 0x88040f);       // mov byte ptr[edi+ecx],al
        WriteInstruction8(tmp, 0x40);            // inc eax
        WriteInstruction24(tmp, 0x83f820);       // cmp eax,20h
        begin_loop_2.WriteJump(tmp, JC::JB);     // jb begin_loop_2

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        jumpBack.WriteJump(tmp, JC::JMP);   // jmp jumpBack

        codespace = tmp;
    }

    // two pops need to be executed either way so prepare that
    uint16_t pops;
    std::memcpy(&pops, addressMapLookupPops, 2);
    {
        PageUnprotect page(logger, addressMapLookupFailMov - 2, 2);
        std::memcpy(addressMapLookupFailMov - 2, &pops, 2);
    }
    {
        PageUnprotect page(logger, addressMapLookupSuccessPush, 3);
        std::memcpy(addressMapLookupSuccessPush, &pops, 2);
        *(addressMapLookupSuccessPush + 2) = 0x51; // push ecx
    }


    {
        // clear out old lookup code
        char* tmp = addressMapLookupCode;
        PageUnprotect page(logger, tmp, lengthMapLookupCodeForDelete);
        for (size_t i = 0; i < lengthMapLookupCodeForDelete; ++i) {
            WriteInstruction8(tmp, 0x90); // nop
        }

        // replace with new lookup code
        BranchHelper1Byte lookup_success;
        BranchHelper1Byte lookup_fail;

        // input is in edx
        // on success: result should be in ecx
        // on fail: result doesn't matter, restores itself
        lookup_success.SetTarget(addressMapLookupSuccessPush);
        lookup_fail.SetTarget(addressMapLookupFailMov - 2);

        tmp = addressMapLookupCode;
        WriteInstruction24(tmp, 0x83fa20);       // cmp edx,20h
        lookup_fail.WriteJump(tmp, JC::JNB);     // jnb lookup_fail
        WriteInstruction24(tmp, 0x8b4dfc);       // mov ecx,dword ptr[ebp-4h]
        WriteInstruction40(tmp, 0x0fb64c1104);   // movzx ecx,byte ptr[ecx+edx+4h]
        WriteInstruction24(tmp, 0x83f920);       // cmp ecx,20h
        lookup_fail.WriteJump(tmp, JC::JNB);     // jnb lookup_fail
        lookup_success.WriteJump(tmp, JC::JMP);  // jmp lookup_success
    }

    if (version == GameVersion::Japanese) {
        // swap the config <-> game mapping for circle and cross to match the English executable
        {
            char* const swap1 = GetCodeAddressJpEn(version, textRegion, 0x6ac8b2 + 1, 0);
            char* const swap2 = GetCodeAddressJpEn(version, textRegion, 0x6ac8ab + 1, 0);
            PageUnprotect page(logger, std::min(swap1, swap2), static_cast<size_t>(std::max(swap1, swap2) - std::min(swap1, swap2) + 1));
            std::swap(*swap1, *swap2);
        }
        {
            char* const swap1 = GetCodeAddressJpEn(version, textRegion, 0x6ac6ee + 1, 0);
            char* const swap2 = GetCodeAddressJpEn(version, textRegion, 0x6ac6f5 + 1, 0);
            PageUnprotect page(logger, std::min(swap1, swap2), static_cast<size_t>(std::max(swap1, swap2) - std::min(swap1, swap2) + 1));
            std::swap(*swap1, *swap2);
        }

        // swap actions 4/5 on mouse config and controller config readin
        for (uint32_t baseaddr : {0x6abdbau, 0x6abfd5u}) {
            BranchHelper4Byte jumpToNewCode;
            BranchHelper4Byte replacedCall;
            BranchHelper1Byte check5;
            BranchHelper1Byte checkdone;
            BranchHelper4Byte jumpBack;
            char* const addr = GetCodeAddressJpEn(version, textRegion, baseaddr, 0);
            char* tmp = codespace;

            int32_t relativeCallAddress;
            std::memcpy(&relativeCallAddress, addr + 1, 4);
            char* const absoluteCallAddress = relativeCallAddress + addr + 5;
            replacedCall.SetTarget(absoluteCallAddress);

            {
                char* injectionSide = addr;
                PageUnprotect page(logger, injectionSide, 5);
                jumpToNewCode.SetTarget(tmp);
                jumpToNewCode.WriteJump(injectionSide, JC::JMP); // jmp jumpToNewCode
                jumpBack.SetTarget(injectionSide);
            }


            replacedCall.WriteJump(tmp, JC::CALL);     // call (replaced function call)
            WriteInstruction24(tmp, 0x8d4dcc);         // lea ecx,[ebp-34h]
            WriteInstruction16(tmp, 0x8b11);           // mov edx,dword ptr[ecx]
            WriteInstruction24(tmp, 0x83fa04);         // cmp edx,4
            check5.WriteJump(tmp, JC::JNE);            // jne check5
            WriteInstruction48(tmp, 0xc70105000000);   // mov dword ptr[ecx],5
            checkdone.WriteJump(tmp, JC::JMP);
            check5.SetTarget(tmp);
            WriteInstruction24(tmp, 0x83fa05);         // cmp edx,5
            checkdone.WriteJump(tmp, JC::JNE);         // jne checkdone
            WriteInstruction48(tmp, 0xc70104000000);   // mov dword ptr[ecx],4
            checkdone.SetTarget(tmp);
            jumpBack.WriteJump(tmp, JC::JMP);          // jmp jumpBack

            codespace = tmp;
        }

        // this is the function that handles the button prompt (re)mapping
        //{
        //    char* tmp = GetCodeAddressJpEn(version, textRegion, 0x6964d5, 0);
        //    PageUnprotect page(logger, tmp, 1);
        //    WriteInstruction8(tmp, 0xcc);
        //}

        // this inits the struct responsible for telling which button to use for the textbox text advance prompts
        // the JP version incorrectly uses the cancel button instead of the confirm button, so swap that around
        {
            char* const tmp = GetCodeAddressJpEn(version, textRegion, 0x699777, 0);
            PageUnprotect page(logger, tmp, 1);
            *tmp = 4;
        }
    }
    // clang-format on

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen2

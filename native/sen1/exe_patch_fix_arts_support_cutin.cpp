#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
static char Flag = 0;

void PatchFixArtsSupportCutin(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;
    char* addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x5d15ad, 0x5d277d);
    char* flagOffInjectPos = GetCodeAddressJpEn(version, textRegion, 0x4b4f5a, 0x4b67aa);
    char* flagOnInjectPos = GetCodeAddressJpEn(version, textRegion, 0x490565, 0x491db5);
    char* texcoordInjectPos1 = GetCodeAddressJpEn(version, textRegion, 0x504e69, 0x506599);
    char* texcoordInjectPos2 = GetCodeAddressJpEn(version, textRegion, 0x504ff3, 0x506723);
    char* configStructPos = GetCodeAddressJpEn(version, textRegion, 0x1302a30, 0x1304dd0);
    char* addressFlag = &Flag;
    char* linkAbilityTriggerPos = GetCodeAddressJpEn(version, textRegion, 0x686907, 0x688407);

    using JC = JumpCondition;

    //{
    //    // force link abilities to always trigger
    //    char* tmp = linkAbilityTriggerPos;
    //    PageUnprotect page(logger, tmp, 3);
    //    WriteInstruction24(tmp, 0xc6c001);
    //}

    {
        // turn on flag when arts support starts
        BranchHelper4Byte jumpBack;
        char* tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<5, PaddingInstruction::Nop>(logger, flagOnInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        WriteInstruction16(tmp, 0xc605); // mov byte ptr[addressFlag],2
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        *tmp = 2;
        ++tmp;
        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack

        codespace = tmp;
    }

    // turn off flag during this function that seems to be called after all link actions
    // this may need further adjustments
    {
        BranchHelper4Byte jumpBack;
        char* tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<5, PaddingInstruction::Nop>(logger, flagOffInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        WriteInstruction16(tmp, 0xc605); // mov byte ptr[addressFlag],0
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        *tmp = 0;
        ++tmp;
        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack

        codespace = tmp;
    }

    // fix texcoords when running at not 1280x720
    {
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte skip_modification;

        char* tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<5, PaddingInstruction::Nop>(logger, texcoordInjectPos1, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // eax, edx okay to use
        WriteInstruction8(tmp, 0xba); // mov edx,addressFlag
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x8a02);           // mov al,byte ptr[edx]
        WriteInstruction16(tmp, 0x3c02);           // cmp al,2
        skip_modification.WriteJump(tmp, JC::JNE); // jne skip_modification

        WriteInstruction40(tmp, 0xb80000a044);   // mov eax,float(1280.0)
        WriteInstruction48(tmp, 0x898554ffffff); // mov dword ptr[ebp-0ach],eax
        WriteInstruction40(tmp, 0xb800003444);   // mov eax,float(720.0)
        WriteInstruction48(tmp, 0x898558ffffff); // mov dword ptr[ebp-0a8h],eax

        skip_modification.SetTarget(tmp);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack

        codespace = tmp;
    }

    {
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte skip_modification;

        char* tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, texcoordInjectPos2, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        // eax, edx okay to use
        WriteInstruction8(tmp, 0xba); // mov edx,addressFlag
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x8a02);           // mov al,byte ptr[edx]
        WriteInstruction16(tmp, 0x3c02);           // cmp al,2
        skip_modification.WriteJump(tmp, JC::JNE); // jne skip_modification

        WriteInstruction24(tmp, 0xc60201); // mov byte ptr[edx],1

        WriteInstruction8(tmp, 0xba); // mov edx,configStructPos
        std::memcpy(tmp, &configStructPos, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x8b12);          // mov edx,dword ptr[edx]
        WriteInstruction16(tmp, 0x85d2);          // test edx,edx
        skip_modification.WriteJump(tmp, JC::JZ); // jz skip_modification

        WriteInstruction56(tmp, 0xc745d80000a044); // mov dword ptr[ebp-0ach+84h],44a00000h
        WriteInstruction24(tmp, 0xd945d8);         // fld dword ptr[ebp-0ach+84h]
        WriteInstruction24(tmp, 0xdb4214);         // fild dword ptr[edx+14h]
        WriteInstruction16(tmp, 0xdec9);           // fmulp ; 1280*height
        WriteInstruction24(tmp, 0xd94588);         // fld dword ptr[ebp-0ach+34h]
        WriteInstruction24(tmp, 0xdb4210);         // fild dword ptr[edx+10h]
        WriteInstruction16(tmp, 0xdec9);           // fmulp ; 720*width
        WriteInstruction16(tmp, 0xdef9);           // fdivp ; (1280*height)/(720*width)
                                                   // ^ multiplication factor for texcoords
        WriteInstruction16(tmp, 0xd9c0);           // fld st(0)
        WriteInstruction48(tmp, 0xd9856cffffff);   // fld dword ptr[ebp-0ach+18h]
        WriteInstruction16(tmp, 0xdec9);           // fmulp
        WriteInstruction48(tmp, 0xd99d6cffffff);   // fstp dword ptr[ebp-0ach+18h]
        WriteInstruction24(tmp, 0xd94590);         // fld dword ptr[ebp-0ach+3ch]
        WriteInstruction16(tmp, 0xdec9);           // fmulp
        WriteInstruction24(tmp, 0xd95590);         // fst dword ptr[ebp-0ach+3ch]
        WriteInstruction24(tmp, 0xd95dd8);         // fstp dword ptr[ebp-0ach+84h]

        skip_modification.SetTarget(tmp);

        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack

        codespace = tmp;
    }

    // clang-format off
    // modify camera x when aspect ratio != 16:9
    {
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte skip_modification;

        char* tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, addressInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // ecx, edx is free
        WriteInstruction16(tmp, 0x8a0d);             // mov cl,byte ptr[addressFlag]
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x84c9);             // test cl,cl
        skip_modification.WriteJump(tmp, JC::JZ);    // jz skip_modification

        WriteInstruction8(tmp, 0xba);                // mov edx,configStructPos
        std::memcpy(tmp, &configStructPos, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x8b12);             // mov edx,dword ptr[edx]
        WriteInstruction16(tmp, 0x85d2);             // test edx,edx
        skip_modification.WriteJump(tmp, JC::JZ);    // jz skip_modification

        WriteInstruction24(tmp, 0xdb4210);           // fild dword ptr[edx+10h]
        WriteInstruction24(tmp, 0xdb4214);           // fild dword ptr[edx+14h]
        WriteInstruction16(tmp, 0xdef9);             // fdivp ; width/height

        WriteInstruction32(tmp, 0xc74424fc);         // mov dword ptr[esp-4h],float(16/9)
        WriteInstruction32(tmp, 0x398ee33f);
        WriteInstruction32(tmp, 0xd94424fc);         // fld dword ptr[esp-4h]
        WriteInstruction16(tmp, 0xdee9);             // fsubp
        WriteInstruction32(tmp, 0xc74424fc);         // mov dword ptr[esp-4h],float(-0.507)
        WriteInstruction32(tmp, 0xc1ca01bf);
        WriteInstruction32(tmp, 0xd94424fc);         // fld dword ptr[esp-4h]
        WriteInstruction16(tmp, 0xdec9);             // fmulp

        WriteInstruction48(tmp, 0xd9857cfeffff);     // fld dword ptr[ebp-1a4h+20h]
        WriteInstruction16(tmp, 0xdec1);             // faddp
        WriteInstruction48(tmp, 0xd99d7cfeffff);     // fstp dword ptr[ebp-1a4h+20h]

        skip_modification.SetTarget(tmp);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        jumpBack.WriteJump(tmp, JC::JMP);          // jmp jumpBack

        codespace = tmp;
    }
    // clang-format on

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen1

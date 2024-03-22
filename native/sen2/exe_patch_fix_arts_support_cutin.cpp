#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
static char Flag = 0;

void PatchFixArtsSupportCutin(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x560beb, 0x56091b);
    char* flagOffInjectPos = GetCodeAddressJpEn(version, textRegion, 0x468091, 0x468251);
    char* flagOnInjectPos = GetCodeAddressJpEn(version, textRegion, 0x433650, 0x433aa0);
    char* texcoordInjectPos = GetCodeAddressJpEn(version, textRegion, 0x4a22a9, 0x4a25b9);
    char* configStructPos = GetCodeAddressJpEn(version, textRegion, 0x10f7cf4, 0x10fc574);
    char* addressFlag = &Flag;
    char* linkAbilityTriggerPos = GetCodeAddressJpEn(version, textRegion, 0x62030a, 0x61fe5a);

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
        char*& tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<10, PaddingInstruction::Nop>(logger, flagOnInjectPos, tmp);
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
    }

    // turn off flag during this function that seems to be called after all link actions
    // this may need further adjustments
    {
        BranchHelper4Byte jumpBack;
        char*& tmp = codespace;
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
    }

    // fix texcoords when running at not 1280x720
    {
        BranchHelper4Byte jumpToNewCode;
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte skip_modification;

        char*& tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, texcoordInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // eax, edx is free
        // xmm0, xmm1, xmm2 need to be fixed up
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
        WriteInstruction24(tmp, 0x8b4210);        // eax,dword ptr[edx+10h] ; width
        WriteInstruction24(tmp, 0x8b5214);        // edx,dword ptr[edx+14h] ; height
        WriteInstruction32(tmp, 0x660f6ec0);      // movd xmm0,eax
        WriteInstruction24(tmp, 0x0f5bc0);        // cvtdq2ps xmm0,xmm0
        WriteInstruction32(tmp, 0x660f6eca);      // movd xmm1,edx
        WriteInstruction24(tmp, 0x0f5bc9);        // cvtdq2ps xmm1,xmm1
        WriteInstruction32(tmp, 0xf30f5ec1);      // divss xmm0,xmm1

        // xmm0 now holds the actual aspect ratio

        WriteInstruction40(tmp, 0xba398ee33f); // mov edx,float(16/9)
        WriteInstruction32(tmp, 0x660f6eca);   // movd xmm1,edx
        WriteInstruction32(tmp, 0xf30f5ec8);   // divss xmm1,xmm0

        // xmm1 now holds (16/9) / (real AR), ie the factor to multiply the horizontal texcoords
        // with

        WriteInstruction40(tmp, 0xb8cdcc4c3d); // mov eax,float(0.05)
        WriteInstruction32(tmp, 0x660f6ec0);   // movd xmm0,eax
        WriteInstruction32(tmp, 0xf30f59c1);   // mulss xmm0,xmm1
        WriteInstruction40(tmp, 0xb8cdcc4c3e); // mov eax,float(0.2)
        WriteInstruction32(tmp, 0x660f6ed0);   // movd xmm2,eax
        WriteInstruction32(tmp, 0xf30f59d1);   // mulss xmm2,xmm1
        WriteInstruction40(tmp, 0xb80000803f); // mov eax,float(1.0)
        WriteInstruction32(tmp, 0x660f6ec8);   // movd xmm1,eax

        skip_modification.SetTarget(tmp);

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack
    }

    // clang-format off
    // modify matrix during arts support
    {
        BranchHelper4Byte jumpBack;
        BranchHelper1Byte skip_modification;

        char*& tmp = codespace;
        const auto injectResult =
            InjectJumpIntoCode<7, PaddingInstruction::Nop>(logger, addressInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);
        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        // ecx, edx, xmm0-2 is free
        // eax contains address to matrix
        WriteInstruction16(tmp, 0x8a0d);           // mov cl,byte ptr[addressFlag]
        std::memcpy(tmp, &addressFlag, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x84c9);           // test cl,cl
        skip_modification.WriteJump(tmp, JC::JZ);  // jz skip_modification

        // fix visibility
        WriteInstruction40(tmp, 0xf30f10402c);     // movss xmm0,dword ptr[eax+2ch]
        WriteInstruction40(tmp, 0xba0000403f);     // mov edx,float(0.75)
        WriteInstruction32(tmp, 0x660f6eca);       // movd xmm1,edx
        WriteInstruction32(tmp, 0xf30f59c1);       // mulss xmm0,xmm1
        WriteInstruction40(tmp, 0xf30f11402c);     // movss dword ptr[eax+2ch],xmm0

        // modify y position
        WriteInstruction40(tmp, 0xf30f104024);     // movss xmm0,dword ptr[eax+24h]
        WriteInstruction40(tmp, 0xbacdcc4c3e);     // mov edx,float(0.2)
        WriteInstruction32(tmp, 0x660f6eca);       // movd xmm1,edx
        WriteInstruction32(tmp, 0xf30f58c1);       // addss xmm0,xmm1
        WriteInstruction40(tmp, 0xf30f114024);     // movss dword ptr[eax+24h],xmm0

        // if aspect ratio != 16:9, shift camera so that model is still at left edge of texture
        // this is pretty silly tbqh but oh well
        WriteInstruction8(tmp, 0xba);              // mov edx,configStructPos
        std::memcpy(tmp, &configStructPos, 4);
        tmp += 4;
        WriteInstruction16(tmp, 0x8b12);           // mov edx,dword ptr[edx]
        WriteInstruction16(tmp, 0x85d2);           // test edx,edx
        skip_modification.WriteJump(tmp, JC::JZ);  // jz skip_modification
        WriteInstruction24(tmp, 0x8b4a10);         // ecx,dword ptr[edx+10h] ; width
        WriteInstruction24(tmp, 0x8b5214);         // edx,dword ptr[edx+14h] ; height
        WriteInstruction32(tmp, 0x660f6ec1);       // movd xmm0,ecx
        WriteInstruction24(tmp, 0x0f5bc0);         // cvtdq2ps xmm0,xmm0
        WriteInstruction32(tmp, 0x660f6eca);       // movd xmm1,edx
        WriteInstruction24(tmp, 0x0f5bc9);         // cvtdq2ps xmm1,xmm1
        WriteInstruction32(tmp, 0xf30f5ec1);       // divss xmm0,xmm1 ; xmm0 now holds the actual aspect ratio
        WriteInstruction40(tmp, 0xba398ee33f);     // mov edx,float(16/9)
        WriteInstruction32(tmp, 0x660f6eca);       // movd xmm1,edx
        WriteInstruction32(tmp, 0xf30f5cc1);       // subss xmm0,xmm1
        WriteInstruction40(tmp, 0xba3333f3be);     // mov edx,float(-0.475)
        WriteInstruction32(tmp, 0x660f6eca);       // movd xmm1,edx
        WriteInstruction32(tmp, 0xf30f59c1);       // mulss xmm0,xmm1
        WriteInstruction40(tmp, 0xf30f104820);     // movss xmm1,dword ptr[eax+20h]
        WriteInstruction32(tmp, 0xf30f58c8);       // addss xmm1,xmm0
        WriteInstruction40(tmp, 0xf30f114820);     // movss dword ptr[eax+20h],xmm1

        skip_modification.SetTarget(tmp);

        jumpBack.WriteJump(tmp, JC::JMP);          // jmp jumpBack
    }
    // clang-format on
}
} // namespace SenLib::Sen2

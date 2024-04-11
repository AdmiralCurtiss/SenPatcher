#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchMusicQueueing(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd) {
    using namespace SenPatcher::x86;

    char* const bgmPlayingCheckInjectAddress =
        GetCodeAddressJpEn(version, textRegion, 0x5e7291, 0x5e82b1);
    char* const bgmFadingReturn2InjectAddress =
        GetCodeAddressJpEn(version, textRegion, 0x459012, 0x459162);

    // Unlike the later games, CS1 does not have a separate audio processing thread, everything
    // happens on the main one. This means we don't have to worry about data races.

    // The function at 0x459110 is our bgm playing check function, it returns a boolean false if the
    // to-enqueue track mismatches and a boolean true if the to-enqueue track matches. Unlike CS3
    // and onwards, this does *not* check for the fade state here, which can be found at [edx+0x30]
    // (as a single byte) if you're setting a breakpoint at 0x459153. Again unlike later games, this
    // only knows two states: Playing (0) and Fading (1). There is no (2) state; after the fade is
    // done it just resets back to (0). A separate flag keeps track of whether a track is playing at
    // all (this is what the check at 0x459139 does).

    // In order to fix this bug here in CS1, we do a similar fix we do in CS3 and later. We let the
    // function at 0x459110 return '2' in the fading case, and then check for != 1 at the call site
    // that decides whether a music track should be enqueued or not.

    {
        char*& tmp = codespace;
        PageUnprotect page(logger, bgmFadingReturn2InjectAddress, 7);

        // reshuffle instructions so it's cleaner to inject
        // this throws away a 'setz al' instruction, we don't need it
        std::array<char, 2> cmp_eax_edi;
        std::memcpy(cmp_eax_edi.data(), bgmFadingReturn2InjectAddress, cmp_eax_edi.size());
        std::memcpy(bgmFadingReturn2InjectAddress + 5, bgmFadingReturn2InjectAddress + 2, 2);

        // inject
        BranchHelper4Byte injectBranch;
        injectBranch.SetTarget(tmp);
        char* injectAddress = bgmFadingReturn2InjectAddress;
        injectBranch.WriteJump(injectAddress, JumpCondition::JMP);
        BranchHelper4Byte returnBranch;
        BranchHelper1Byte returnBranchShort;
        returnBranch.SetTarget(injectAddress);

        // re-handle the comparison we overwrote
        BranchHelper1Byte return0;
        std::memcpy(tmp, cmp_eax_edi.data(), cmp_eax_edi.size()); // cmp eax,edi
        tmp += cmp_eax_edi.size();
        return0.WriteJump(tmp, JumpCondition::JNE);

        // check fade flag
        BranchHelper1Byte return2;
        WriteInstruction24(tmp, 0x8b4614); // mov eax,dword ptr[esi + 14h]
        WriteInstruction24(tmp, 0x8b0498); // mov eax,dword ptr[eax + ebx*4h]
        WriteInstruction24(tmp, 0x8a4030); // mov al,byte ptr[eax + 30h]
        WriteInstruction16(tmp, 0x84c0);   // test al,al
        return2.WriteJump(tmp, JumpCondition::JNZ);

        WriteInstruction16(tmp, 0xb001); // mov al,1
        returnBranchShort.WriteJump(tmp, JumpCondition::JMP);

        return2.SetTarget(tmp);
        WriteInstruction16(tmp, 0xb002); // mov al,2
        returnBranchShort.WriteJump(tmp, JumpCondition::JMP);

        return0.SetTarget(tmp);
        Emit_XOR_R32_R32(tmp, R32::EAX, R32::EAX);
        returnBranchShort.SetTarget(tmp);
        returnBranch.WriteJump(tmp, JumpCondition::JMP);
    }

    {
        // We're replacing:
        // test al,al
        // jz [continueEnqueueTargetAddress]
        // with:
        // cmp al,1
        // jne [continueEnqueueTargetAddress]
        char* tmp = bgmPlayingCheckInjectAddress;
        PageUnprotect page(logger, tmp, 3);
        WriteInstruction24(tmp, 0x3c0175);
    }
}
} // namespace SenLib::Sen1

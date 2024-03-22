#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchRemoveDebugLeftovers(SenPatcher::Logger& logger,
                               char* textRegion,
                               GameVersion version,
                               char*& codespace,
                               char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* begin = GetCodeAddressJpEn(version, textRegion, 0x4d1479, 0x4d17a9);
    char* end = GetCodeAddressJpEn(version, textRegion, 0x4d154f, 0x4d187f);

    // this appears to be some kind of debug/dev code to screenshot the framebuffer on the monster
    // analysis screen
    // unfortunately it softlocks the game if triggered and the outpath (hardcoded to the z: drive)
    // is not writable, so just jump around this entire block and don't even pretend it's useful...
    {
        char* tmp = begin;
        PageUnprotect page(logger, tmp, static_cast<int>(end - begin));

        // nop out the code in case some branch still exists into it -- i don't think so but
        // y'know...
        while (tmp < end) {
            *tmp++ = static_cast<char>(0x90); // nop
        }

        tmp = begin;
        BranchHelper4Byte jmp; // jmp past this mess
        jmp.SetTarget(end - 2);
        jmp.WriteJump(tmp, JumpCondition::JMP);
        tmp = end - 2;
        WriteInstruction16(tmp, 0x31c0); // xor eax,eax
    }

    // patch the bizarre and apparently behavior that messes with the music timing when left shift
    // or left ctrl are held
    char* const MultiplierWhenLCrtlHeld =
        GetCodeAddressJpEn(version, textRegion, 0x581ba6, 0x581f66);
    char* const MultiplierWhenLShiftHeld =
        GetCodeAddressJpEn(version, textRegion, 0x581bbc, 0x581f7c);
    const uint32_t float1 = 0x3f800000;
    {
        PageUnprotect pageLCtrl(logger,
                                MultiplierWhenLCrtlHeld,
                                MultiplierWhenLShiftHeld - MultiplierWhenLCrtlHeld + 4);
        std::memcpy(MultiplierWhenLCrtlHeld, &float1, 4);
        std::memcpy(MultiplierWhenLShiftHeld, &float1, 4);
    }
}
} // namespace SenLib::Sen2

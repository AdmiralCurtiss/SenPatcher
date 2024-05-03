#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen1 {
void RemoveTurboAutoSkip(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* addressJumpBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4d6411, 0x4d7c61);
    char* addressJumpBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4f0a2f, 0x4f224f);

    {
        PageUnprotect page(logger, addressJumpBattleAnimationAutoSkip, 1);
        *addressJumpBattleAnimationAutoSkip = static_cast<char>(0xeb); // jmp
    }
    {
        PageUnprotect page(logger, addressJumpBattleResultsAutoSkip, 2);
        *addressJumpBattleResultsAutoSkip = static_cast<char>(0x90);       // nop
        *(addressJumpBattleResultsAutoSkip + 1) = static_cast<char>(0xe9); // jmp
    }
}
} // namespace SenLib::Sen1

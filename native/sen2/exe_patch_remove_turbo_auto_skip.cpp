#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen2 {
void RemoveTurboAutoSkip(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* addressJumpBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x479b1b, 0x479c8b);
    char* addressJumpBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4929ba, 0x492bea);
    char* addressJumpBattleSomethingAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4845ae, 0x48474e);
    char* addressJumpBattleStartAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4836eb, 0x48388b);

    {
        PageUnprotect page(logger, addressJumpBattleAnimationAutoSkip, 1);
        *addressJumpBattleAnimationAutoSkip = static_cast<char>(0xeb); // jmp
    }
    {
        PageUnprotect page(logger, addressJumpBattleStartAutoSkip, 1);
        *addressJumpBattleStartAutoSkip = static_cast<char>(0xeb); // jmp
    }
    {
        PageUnprotect page(logger, addressJumpBattleSomethingAutoSkip, 1);
        *addressJumpBattleSomethingAutoSkip = static_cast<char>(0xeb); // jmp
    }
    {
        PageUnprotect page(logger, addressJumpBattleResultsAutoSkip, 2);
        *addressJumpBattleResultsAutoSkip = static_cast<char>(0x90);       // nop
        *(addressJumpBattleResultsAutoSkip + 1) = static_cast<char>(0xe9); // jmp
    }
}
} // namespace SenLib::Sen2

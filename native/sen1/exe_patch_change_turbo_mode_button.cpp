#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen1 {
void ChangeTurboModeButton(SenPatcher::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           char*& codespace,
                           char* codespaceEnd,
                           int turboModeButton) {
    if (turboModeButton < 0 || turboModeButton > 0xF) {
        logger.Log("Invalid turbo mode button, skipping patch.\n");
        return;
    }
    const char button = static_cast<char>(turboModeButton);

    using namespace SenPatcher::x86;
    char* const addressButtonBattleAnimationAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4d6409, 0x4d7c59);
    char* const addressButtonBattleResultsAutoSkip =
        GetCodeAddressJpEn(version, textRegion, 0x4f0a27, 0x4f2247);
    char* const addressButtonTurboMode =
        GetCodeAddressJpEn(version, textRegion, 0x48179a, 0x48300a);

    {
        PageUnprotect page(logger, addressButtonBattleAnimationAutoSkip, 1);
        *addressButtonBattleAnimationAutoSkip = button;
    }
    {
        PageUnprotect page(logger, addressButtonBattleResultsAutoSkip, 1);
        *addressButtonBattleResultsAutoSkip = button;
    }
    {
        PageUnprotect page(logger, addressButtonTurboMode, 1);
        *addressButtonTurboMode = button;
    }
}
} // namespace SenLib::Sen1

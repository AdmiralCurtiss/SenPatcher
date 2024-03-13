#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void AllowSwitchToNightmare(SenPatcher::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* setMaxDifficultyIndexPos =
        GetCodeAddressJpEn(version, textRegion, 0x14042a667, 0x140436347) + 1;
    char* switchFromNightmareWarning =
        GetCodeAddressJpEn(version, textRegion, 0x140236118, 0x14023c65d) + 1;

    // allow nightmare to be selected when difficulty is currently not nightmare
    {
        PageUnprotect page(logger, setMaxDifficultyIndexPos, 4);
        const uint32_t value = 5;
        std::memcpy(setMaxDifficultyIndexPos, &value, 4);
    }

    // don't show warning when trying to switch away from nightmare
    {
        PageUnprotect page(logger, switchFromNightmareWarning, 4);
        const uint32_t value = 0x1e4;
        std::memcpy(switchFromNightmareWarning, &value, 4);
    }
}
} // namespace SenLib::Sen3

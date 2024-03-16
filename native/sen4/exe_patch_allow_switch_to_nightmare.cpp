#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void AllowSwitchToNightmare(SenPatcher::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* setMaxDifficultyIndexPos =
        GetCodeAddressJpEn(version, textRegion, 0x140486117, 0x140488647) + 1;
    char* switchFromNightmareWarning1 =
        GetCodeAddressJpEn(version, textRegion, 0x140210076, 0x140212326) + 1;
    char* switchFromNightmareWarning2 =
        GetCodeAddressJpEn(version, textRegion, 0x1403d5842, 0x1403d7ac2) + 1;

    // allow nightmare to be selected when difficulty is currently not nightmare
    {
        PageUnprotect page(logger, setMaxDifficultyIndexPos, 4);
        const uint32_t value = 5;
        std::memcpy(setMaxDifficultyIndexPos, &value, 4);
    }

    // don't show warning when trying to switch away from nightmare
    {
        PageUnprotect page(logger, switchFromNightmareWarning1, 4);
        const uint32_t value = 0x224;
        std::memcpy(switchFromNightmareWarning1, &value, 4);
    }
    {
        PageUnprotect page(logger, switchFromNightmareWarning2, 4);
        const uint32_t value = 0x224;
        std::memcpy(switchFromNightmareWarning2, &value, 4);
    }
}
} // namespace SenLib::Sen4

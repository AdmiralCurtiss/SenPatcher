#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void AllowSwitchToNightmare(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* setMaxDifficultyIndexPos = GetCodeAddressJpEn(version,
                                                        textRegion,
                                                        Addresses{.Jp121 = 0x140486117,
                                                                  .En121 = 0x140488647,
                                                                  .Jp122 = 0x140486077,
                                                                  .En122 = 0x1404885a7})
                                     + 1;
    char* switchFromNightmareWarning1 = GetCodeAddressJpEn(version,
                                                           textRegion,
                                                           Addresses{.Jp121 = 0x140210076,
                                                                     .En121 = 0x140212326,
                                                                     .Jp122 = 0x14020ffd6,
                                                                     .En122 = 0x140212286})
                                        + 1;
    char* switchFromNightmareWarning2 = GetCodeAddressJpEn(version,
                                                           textRegion,
                                                           Addresses{.Jp121 = 0x1403d5842,
                                                                     .En121 = 0x1403d7ac2,
                                                                     .Jp122 = 0x1403d57a2,
                                                                     .En122 = 0x1403d7a22})
                                        + 1;

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

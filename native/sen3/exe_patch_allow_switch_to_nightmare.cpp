#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void AllowSwitchToNightmare(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* setMaxDifficultyIndexPos = GetCodeAddressJpEn(version,
                                                        textRegion,
                                                        Addresses{.Jp106 = 0x14042a667,
                                                                  .En106 = 0x140436347,
                                                                  .Jp107 = 0x14042a5e7,
                                                                  .En107 = 0x1404362c7})
                                     + 1;
    char* switchFromNightmareWarning = GetCodeAddressJpEn(version,
                                                          textRegion,
                                                          Addresses{.Jp106 = 0x140236118,
                                                                    .En106 = 0x14023c65d,
                                                                    .Jp107 = 0x140236098,
                                                                    .En107 = 0x14023c5dd})
                                       + 1;

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

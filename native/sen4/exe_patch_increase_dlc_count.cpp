#include "exe_patch.h"

#include <algorithm>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchIncreaseDlcCount(PatchExecData& execData, uint32_t newLimit) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* dlcLimitAddress = GetCodeAddressJpEn(version,
                                               textRegion,
                                               Addresses{.Jp121 = 0x1403c42ea,
                                                         .En121 = 0x1403c65fa,
                                                         .Jp122 = 0x1403c424a,
                                                         .En122 = 0x1403c655a})
                            + 2;

    uint32_t oldLimit;
    std::memcpy(&oldLimit, dlcLimitAddress, 4);
    uint32_t dlcLimitValue = std::clamp(newLimit, oldLimit + 1, uint32_t(0x1000)) - 1;
    PageUnprotect page(logger, dlcLimitAddress, 4);
    std::memcpy(dlcLimitAddress, &dlcLimitValue, 4);
}
} // namespace SenLib::Sen4

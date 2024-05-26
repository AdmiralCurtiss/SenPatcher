#include "exe_patch.h"

#include <algorithm>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchIncreaseDlcCount(HyoutaUtils::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           uint32_t newLimit) {
    using namespace SenPatcher::x64;
    char* dlcLimitAddress = GetCodeAddressJpEn(version, textRegion, 0x14039fa4a, 0x1403a998b);

    uint32_t oldLimit;
    std::memcpy(&oldLimit, dlcLimitAddress, 4);
    uint32_t dlcLimitValue = std::clamp(newLimit, oldLimit + 1, uint32_t(0x1000)) - 1;
    PageUnprotect page(logger, dlcLimitAddress, 4);
    std::memcpy(dlcLimitAddress, &dlcLimitValue, 4);
}
} // namespace SenLib::Sen3

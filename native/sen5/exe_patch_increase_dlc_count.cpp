#include "exe_patch.h"

#include <algorithm>
#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchIncreaseDlcCount(HyoutaUtils::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           uint32_t newLimit) {
    using namespace SenPatcher::x64;
    char* dlcLimitAddress = GetCodeAddressEn(version, textRegion, 0x140556935);

    uint32_t oldLimit;
    std::memcpy(&oldLimit, dlcLimitAddress, 4);
    uint32_t dlcLimitValue = std::clamp(newLimit, oldLimit + 1, uint32_t(0x1000)) - 1;
    PageUnprotect page(logger, dlcLimitAddress, 4);
    std::memcpy(dlcLimitAddress, &dlcLimitValue, 4);
}
} // namespace SenLib::Sen5

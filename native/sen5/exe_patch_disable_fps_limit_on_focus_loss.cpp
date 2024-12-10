#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchDisableFpsLimitOnFocusLoss(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* isInBackgroundFlag1 = GetCodeAddressEn(version, textRegion, 0x140972e2a) + 3;

    // tell the fps limiting code that we're in the foreground always
    // for reference, this value is checked at 0x140882fb3 (v1.1.4)
    {
        char* tmp = isInBackgroundFlag1;
        PageUnprotect page(logger, tmp, 1);
        *tmp = 0;
    }
}
} // namespace SenLib::Sen5

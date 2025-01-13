#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchShowMouseCursor(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* showCursorPos = GetCodeAddressJpEn(version,
                                             textRegion,
                                             Addresses{.Jp106 = 0x140599734,
                                                       .En106 = 0x1405a5ae4,
                                                       .Jp107 = 0x1405996b4,
                                                       .En107 = 0x1405a5a64});

    // remove call to ShowCursor(0)
    {
        char* tmp = showCursorPos;
        PageUnprotect page(logger, tmp, 8);
        for (size_t i = 0; i < 8; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen3

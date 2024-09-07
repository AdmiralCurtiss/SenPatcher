#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void PatchShowMouseCursor(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* jumpPos = GetCodeAddressSteamGog(version, textRegion, 0x40d843, 0x40c703);

    // always jump over call to SetCursor(0)
    {
        PageUnprotect page(logger, jumpPos, 1);
        *jumpPos = static_cast<char>(0xeb); // jnz -> jmp
    }
}
} // namespace SenLib::TX

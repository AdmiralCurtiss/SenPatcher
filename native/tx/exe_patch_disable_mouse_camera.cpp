#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void PatchDisableMouseCamera(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* captureMouseCursorPos1 = GetCodeAddressSteamGog(version, textRegion, 0x47cd99, 0x47b7b9);
    char* captureMouseCursorPos2 = GetCodeAddressSteamGog(version, textRegion, 0x47d05d, 0x47ba7d);

    // change function that captures the mouse cursor to not do that
    {
        PageUnprotect page(logger, captureMouseCursorPos1, 2);
        *captureMouseCursorPos1 = static_cast<char>(0x90);       // nop
        *(captureMouseCursorPos1 + 1) = static_cast<char>(0xe9); // jnz -> jmp
    }
    {
        PageUnprotect page(logger, captureMouseCursorPos2, 2);
        *captureMouseCursorPos2 = static_cast<char>(0x90);       // nop
        *(captureMouseCursorPos2 + 1) = static_cast<char>(0xe9); // jnz -> jmp
    }
}
} // namespace SenLib::TX

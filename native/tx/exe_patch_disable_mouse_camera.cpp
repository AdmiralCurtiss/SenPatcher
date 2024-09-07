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
    char* handleMouseCamPos = GetCodeAddressSteamGog(version, textRegion, 0x47cd99, 0x47b7b9);

    // change function that handles mouse camera to not do that
    {
        PageUnprotect page(logger, handleMouseCamPos, 2);
        *handleMouseCamPos = static_cast<char>(0x90);       // nop
        *(handleMouseCamPos + 1) = static_cast<char>(0xe9); // jnz -> jmp
    }
}
} // namespace SenLib::TX

#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstring>

#include "util/memread.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void PatchDisableCameraAutoCenter(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;
    using namespace HyoutaUtils::MemRead;

    char* codespace = execData.Codespace;

    // this is the slow sway back to neutral while moving
    char* const addressBranch1 = GetCodeAddressSteamGog(version, textRegion, 0x47d6fb, 0x47c11b);

    // this is the vertical snap while dashing
    char* const addressBranch2 = GetCodeAddressSteamGog(version, textRegion, 0x47d52b, 0x47bf4b);

    // more notes:
    // - always taking the branch at 0x47c8ee takes manual camera control away completely.
    //   (resetting the camera still works though)

    {
        // jz -> jmp
        char* tmp = addressBranch1;
        PageUnprotect page(logger, tmp, 2);
        *tmp++ = static_cast<char>(0x90);
        *tmp++ = static_cast<char>(0xe9);
    }
    {
        // jz -> jmp
        char* tmp = addressBranch2;
        PageUnprotect page(logger, tmp, 1);
        *tmp++ = static_cast<char>(0xeb);
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::TX

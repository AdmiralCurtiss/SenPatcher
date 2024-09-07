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
    char* handleMouseCamPos1 = GetCodeAddressSteamGog(version, textRegion, 0x47cdb0, 0x47b7d0);
    char* handleMouseCamPos2 = GetCodeAddressSteamGog(version, textRegion, 0x47cfed, 0x47ba0d);

    // nop out the entire code block that's responsible for the mouse camera handling, except for
    // the part at the end that sets the flag to hide the in-game mouse cursor
    {
        char* start = handleMouseCamPos1;
        char* end = handleMouseCamPos2;
        PageUnprotect page(logger, start, static_cast<size_t>(end - start));
        for (char* a = start; a < end; ++a) {
            *a = static_cast<char>(0x90); // nop
        }

        // for efficiency, jump over the block too
        BranchHelper4Byte jmp;
        jmp.SetTarget(end);
        jmp.WriteJump(start, JumpCondition::JMP);
    }
}
} // namespace SenLib::TX

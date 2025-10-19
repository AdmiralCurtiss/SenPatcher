#include "exe_patch.h"

#include <array>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void PatchMSAA(PatchExecData& execData, int msaa) {
    // vanilla game forces 4x MSAA, make that configurable
    char msaa_byte = 0;
    if (msaa < 0) {
        // patch disabled
        return;
    }
    if (msaa == 0 || msaa == 1) {
        msaa_byte = 0;
    } else if (msaa == 2 || msaa == 4 || msaa == 8 || msaa == 16) {
        msaa_byte = static_cast<char>(msaa);
    } else {
        // invalid value configured
        return;
    }

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    logger.Log("Forcing MSAA to ").LogInt(msaa).Log("x\n");

    using namespace SenPatcher::x86;

    // render buffer setup stuff
    char* pos1 = GetCodeAddressSteamGog(version, textRegion, 0x575e54, 0x574424) + 1;
    char* pos2 = GetCodeAddressSteamGog(version, textRegion, 0x575e96, 0x574466) + 1;
    char* pos3 = GetCodeAddressSteamGog(version, textRegion, 0x575eee, 0x5744be) + 1;
    char* pos4 = GetCodeAddressSteamGog(version, textRegion, 0x575f32, 0x574502) + 1;
    char* pos5 = GetCodeAddressSteamGog(version, textRegion, 0x575f95, 0x574565) + 2;
    char* pos6 = GetCodeAddressSteamGog(version, textRegion, 0x575fe7, 0x5745b7) + 1;
    char* pos7 = GetCodeAddressSteamGog(version, textRegion, 0x57620a, 0x5747da) + 1;
    char* pos8 = GetCodeAddressSteamGog(version, textRegion, 0x57625a, 0x57482a) + 1;
    char* pos9 = GetCodeAddressSteamGog(version, textRegion, 0x5762a6, 0x574876) + 1;
    for (char* pos : std::array<char*, 9>({pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9})) {
        PageUnprotect page(logger, pos, 1);
        *pos = msaa_byte;
    }

    // TODO? The CS games also check MSAA in a separate function,
    //  in CS2 it's at 0x787bc4, in TX GOG the equivalent is at 0x72df84.
    //  I'm not sure what exactly that value is being used for there though,
    //  and there's no obvious equivalent to put it in TX...
}
} // namespace SenLib::TX

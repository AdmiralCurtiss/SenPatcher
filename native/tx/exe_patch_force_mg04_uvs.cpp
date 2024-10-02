#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void ForceMG04UVs(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;

    // the English PC texture for the gate of avalon minigame character names has shifted UV
    // coordinates that the code compensates for, instead of being aligned in a nice grid.
    // the English Switch texture does not have this. so if the Switch textures are used, we
    // need to disable this compensation
    char* const addressGateOfAvalonCharSelect =
        GetCodeAddressSteamGog(version, textRegion, 0x5cec4d, 0x5cd0ed);
    static constexpr size_t lengthGateOfAvalonCharSelect = 6;

    // steam and gog use different internal values for the languages, for some reason
    const uint32_t targetLanguage = (version == GameVersion::Steam) ? 1u : 0u;

    {
        char* tmp = addressGateOfAvalonCharSelect;
        PageUnprotect page(logger, tmp, lengthGateOfAvalonCharSelect);
        std::memset(tmp, 0x90, lengthGateOfAvalonCharSelect);
        Emit_MOV_R32_IMM32(tmp, R32::EAX, targetLanguage);
    }
}
} // namespace SenLib::TX

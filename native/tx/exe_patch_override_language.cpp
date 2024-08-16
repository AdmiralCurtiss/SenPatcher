#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void OverrideLanguage(PatchExecData& execData, bool useJapanese) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x40b4da, 0x40a40a);
    constexpr size_t replaceLength = 0x19;

    {
        PageUnprotect page(logger, entryPoint, replaceLength);

        // mov dword ptr[language_int_address],eax
        std::array<char, 6> store_language_value;
        std::memcpy(store_language_value.data(), entryPoint, store_language_value.size());

        // nop out the old code
        std::memset(entryPoint, 0x90, replaceLength);

        // steam and gog use different internal values for the languages, for some reason
        const uint32_t targetLanguage =
            (version == GameVersion::Steam) ? (useJapanese ? 1u : 0u) : (useJapanese ? 0u : 1u);

        char* tmp = entryPoint;
        Emit_MOV_R32_IMM32(tmp, R32::EAX, targetLanguage);
        std::memcpy(tmp, store_language_value.data(), store_language_value.size());
    }
}
} // namespace SenLib::TX

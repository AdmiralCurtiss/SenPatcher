#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "modload/loaded_mods.h"

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen2 {
void AddSenPatcherVersionToTitle(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed) {
    using namespace SenPatcher::x86;
    char* pushAddressVersionTitle = GetCodeAddressJpEn(version, textRegion, 0x62a74c, 0x62a2dc);
    char* pushAddressVersionConsole = GetCodeAddressJpEn(version, textRegion, 0x578911, 0x578c81);
    char* pushAddressVersionCrashRpt = GetCodeAddressJpEn(version, textRegion, 0x425e88, 0x426098);

    // add indicator to the title screen that we're running a modified executable

    uint32_t addressVersionString;
    std::memcpy(&addressVersionString, pushAddressVersionTitle, 4);

    // copy the title screen string into our codespace and expand with senpatcher version
    uint32_t addressNewVersionString = std::bit_cast<uint32_t>(codespace);
    {
        const char* tmp = std::bit_cast<const char*>(addressVersionString);
        while (*tmp != 0) {
            *codespace = *tmp;
            ++codespace;
            ++tmp;
        }
    }
    char* addressNativeVersion = codespace - 5;
    constexpr char senpatcherVersionString[] = "  SenPatcher " SENPATCHER_VERSION;
    std::memcpy(codespace, senpatcherVersionString, sizeof(senpatcherVersionString));
    codespace += (sizeof(senpatcherVersionString) - 1);
    SenLib::ModLoad::AppendLoadedModInfo(codespace, loadedModsData, assetFixCreatingFailed);
    *codespace = 0;
    ++codespace;

    uint32_t addressNewVersionStringShort = std::bit_cast<uint32_t>(codespace);
    {
        const char* tmp = std::bit_cast<const char*>(addressNativeVersion);
        while (*tmp != ' ') {
            *codespace = *tmp;
            ++codespace;
            ++tmp;
        }
    }
    std::memcpy(codespace, senpatcherVersionString + 1, sizeof(senpatcherVersionString) - 1);
    codespace += (sizeof(senpatcherVersionString) - 1);

    {
        PageUnprotect page(logger, pushAddressVersionTitle, 4);
        std::memcpy(pushAddressVersionTitle, &addressNewVersionString, 4);
    }
    {
        PageUnprotect page(logger, pushAddressVersionConsole, 4);
        std::memcpy(pushAddressVersionConsole, &addressNewVersionString, 4);
    }
    {
        PageUnprotect page(logger, pushAddressVersionCrashRpt, 4);
        std::memcpy(pushAddressVersionCrashRpt, &addressNewVersionStringShort, 4);
    }
}
} // namespace SenLib::Sen2

#include "exe_patch.h"

#include <bit>
#include <cassert>
#include <cstring>

#include "modload/loaded_mods.h"

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen1 {
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;
    char* pushAddressVersionTitle = GetCodeAddressJpEn(version, textRegion, 0x68e93d, 0x69042d);

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
    constexpr char senpatcherVersionString[] = "  SenPatcher " SENPATCHER_VERSION;
    std::memcpy(codespace, senpatcherVersionString, sizeof(senpatcherVersionString));
    codespace += (sizeof(senpatcherVersionString) - 1);
    SenLib::ModLoad::AppendLoadedModInfo(
        codespace,
        loadedModsData,
        [](const SenPatcher::P3AFileInfo& fi) {
            return memcmp("data/", fi.Filename.data(), 5) == 0;
        },
        assetFixCreatingFailed);
    *codespace = 0;
    ++codespace;

    {
        PageUnprotect page(logger, pushAddressVersionTitle, 4);
        std::memcpy(pushAddressVersionTitle, &addressNewVersionString, 4);
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen1

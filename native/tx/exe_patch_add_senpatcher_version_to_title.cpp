#include "exe_patch.h"

#include <array>
#include <bit>
#include <cassert>
#include <cstring>

#include "modload/loaded_mods.h"

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::TX {
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed) {
    // This attaches the SenPatcher version string to the 'Quit Game' option on the title screen,
    // displayed when hovering over that option.

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;
    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x658f06, 0x657416);
    char* const addressVersionString =
        GetCodeAddressSteamGog(version, textRegion, 0x9ed34c, 0x9ebf28);

    // copy the version string into our codespace and expand with senpatcher version
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
            return strncmp("_senpatcher_version.txt", fi.Filename.data(), fi.Filename.size()) != 0
                   && strncmp("senpatcher_mod.ini", fi.Filename.data(), fi.Filename.size()) != 0;
        },
        assetFixCreatingFailed);
    *codespace = 0;
    ++codespace;

    const auto inject = InjectJumpIntoCode<5>(logger, entryPoint, codespace);

    WriteInstruction8(codespace, 0x57); // push edi
    WriteInstruction8(codespace, 0x68); // push addressNewVersionString
    std::memcpy(codespace, &addressNewVersionString, 4);
    codespace += 4;
    WriteInstruction16(codespace, 0x6a00); // push 0

    BranchHelper4Byte jumpBack;
    jumpBack.SetTarget(inject.JumpBackAddress);
    jumpBack.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}
} // namespace SenLib::TX

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
    // this doesn't work yet
    return;

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;
    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0, 0x65811f);
    char* const createUiStringFunc = GetCodeAddressSteamGog(version, textRegion, 0, 0x680170);
    char* const addressVersionString = GetCodeAddressSteamGog(version, textRegion, 0, 0x9ebf28);

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
        [](const SenPatcher::P3AFileInfo& fi) { return true; },
        assetFixCreatingFailed);
    *codespace = 0;
    ++codespace;

    const auto inject = InjectJumpIntoCode<5>(logger, entryPoint, codespace);

    WriteInstruction16(codespace, 0x6aff);           // push -1
    WriteInstruction16(codespace, 0x6a00);           // push 0
    WriteInstruction16(codespace, 0x6a00);           // push 0
    WriteInstruction16(codespace, 0x6a00);           // push 0
    WriteInstruction8(codespace, 0x51);              // push ecx, just to advance esp
    WriteInstruction56(codespace, 0xc7042400008041); // mov dword ptr[esp],float(16)
    WriteInstruction8(codespace, 0x68);              // push addressNewVersionString
    std::memcpy(codespace, &addressNewVersionString, 4);
    codespace += 4;
    WriteInstruction16(codespace, 0x6a41); // push 0x41

    // err what do we put in ecx here? we want a UIScript instance but I don't see one...?

    BranchHelper4Byte createUiString;
    createUiString.SetTarget(createUiStringFunc);
    createUiString.WriteJump(codespace, JumpCondition::CALL);

    // finish up function
    Emit_POP_R32(codespace, R32::ESI);
    Emit_RET(codespace);

    execData.Codespace = codespace;
}
} // namespace SenLib::TX

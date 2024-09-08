#include "exe_patch.h"

#include <bit>
#include <cassert>
#include <cstring>

#include "modload/loaded_mods.h"

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen4 {
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;
    char* entryPoint = GetCodeAddressJpEn(version, textRegion, 0x14047b423, 0x14047d556);
    char* rdxLoad = GetCodeAddressJpEn(version, textRegion, 0x14047b43a, 0x14047d56d);
    char* codespaceBegin = codespace;

    // safe to clobber: RCX, RDX (needs to hold string result), R9

    // get the current title screen string
    uint32_t offset;
    std::memcpy(&offset, rdxLoad + 3, 4);
    const char* originalTitleString = rdxLoad + 7 + offset;

    // copy the title screen string into our codespace and expand with senpatcher version
    {
        const char* tmp = originalTitleString;
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

    // inject a jump to codespace
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RDX, codespace);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    // load the title screen string into RDX
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(codespaceBegin));

    // go back
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // remove the old load of rdx
    {
        char* tmp = rdxLoad;
        PageUnprotect page(logger, rdxLoad, 7);
        for (size_t i = 0; i < 7; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen4

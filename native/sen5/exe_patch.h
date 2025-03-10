#pragma once

#include <cassert>
#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen5 {
enum class GameVersion {
    En114,
    En115,
};

struct PatchExecData {
    HyoutaUtils::Logger* Logger;
    char* TextRegion;
    GameVersion Version;
    char* Codespace;
    char* CodespaceEnd;
};

struct Addresses {
    uint64_t En114 = 0;
    uint64_t En115 = 0;
};

inline char* GetCodeAddressEn(GameVersion version, char* textRegion, Addresses addresses) {
    uint64_t addr = 0;
    switch (version) {
        case GameVersion::En114: addr = addresses.En114; break;
        case GameVersion::En115: addr = addresses.En115; break;
        default: break;
    }
    assert(addr != 0);
    return textRegion + (addr - 0x140001000u);
}

void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void PatchMusicQueueing(PatchExecData& execData);
void PatchDisableMouseCapture(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchDisableFpsLimitOnFocusLoss(PatchExecData& execData);
void PatchBrokenSprintf(PatchExecData& execData);

// fix the crash that happens when opening the Costumes menu
// when you have non-installed DLC costumes equipped
// (the game will just use the base model instead)
void PatchDlcCostumeCrash(PatchExecData& execData);

// changes the number of DLCs the game checks for
void PatchIncreaseDlcCount(PatchExecData& execData, uint32_t newLimit);

// removes the already-used checks for custom DLC. official DLC is unaffected.
void PatchCustomDlcMultiuse(PatchExecData& execData);

// removes the check that disables using of DLC items containing costumes if the corresponding
// character is not in the party yet for custom DLC.
void PatchCustomDlcIfNotInParty(PatchExecData& execData);

} // namespace SenLib::Sen5

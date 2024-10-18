#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen3 {
enum class GameVersion {
    Japanese,
    English,
};

struct PatchExecData {
    HyoutaUtils::Logger* Logger;
    char* TextRegion;
    GameVersion Version;
    char* Codespace;
    char* CodespaceEnd;
};

inline char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint64_t addressJp,
                                uint64_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x140001000u)
                                               : (addressEn - 0x140001000u));
}

void DeglobalizeMutexes(PatchExecData& execData);
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void DisableCrashReporter(PatchExecData& execData);
void FixInGameButtonMappingValidity(PatchExecData& execData);
void PatchTurboMode(PatchExecData& execData, bool makeToggle, bool adjustTimersForTurbo);
void AllowSwitchToNightmare(PatchExecData& execData);
void SwapBrokenMasterQuartzValuesForDisplay(PatchExecData& execData);
void PatchDisableMouseCapture(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchDisablePauseOnFocusLoss(PatchExecData& execData);
void PatchForceXInput(PatchExecData& execData);
void PatchFixControllerMappings(PatchExecData& execData);
void PatchMusicQueueing(PatchExecData& execData);

// fix the crash that happens when opening the Costumes menu
// when you have non-installed DLC costumes equipped
// (the game will just use the base model instead)
void PatchDlcCostumeCrash(PatchExecData& execData);

// fix the out-of-bounds write when the english and french DLC files
// disagree on the number of entries, which can easily happen with mods
void PatchDlcLoadFrenchBounds(PatchExecData& execData);

// changes the number of DLCs the game checks for
void PatchIncreaseDlcCount(PatchExecData& execData, uint32_t newLimit);

// removes the already-used checks for custom DLC. official DLC is unaffected.
void PatchCustomDlcMultiuse(PatchExecData& execData);

} // namespace SenLib::Sen3

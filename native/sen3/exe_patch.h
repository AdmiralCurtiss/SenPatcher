#pragma once

#include <cassert>
#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen3 {
enum class GameVersion {
    Jp106,
    En106,
    Jp107,
    En107,
};

inline bool IsGameVersionJp(GameVersion version) {
    return version == GameVersion::Jp106 || version == GameVersion::Jp107;
}

struct PatchExecData {
    HyoutaUtils::Logger* Logger;
    char* TextRegion;
    GameVersion Version;
    char* Codespace;
    char* CodespaceEnd;
};

struct Addresses {
    uint64_t Jp106 = 0;
    uint64_t En106 = 0;
    uint64_t Jp107 = 0;
    uint64_t En107 = 0;
};

inline char* GetCodeAddressJpEn(GameVersion version, char* textRegion, Addresses addresses) {
    uint64_t addr = 0;
    switch (version) {
        case GameVersion::Jp106: addr = addresses.Jp106; break;
        case GameVersion::En106: addr = addresses.En106; break;
        case GameVersion::Jp107: addr = addresses.Jp107; break;
        case GameVersion::En107: addr = addresses.En107; break;
        default: break;
    }
    assert(addr != 0);
    return textRegion + (addr - 0x140001000u);
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
void PatchDisableMotionBlur(PatchExecData& execData);

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

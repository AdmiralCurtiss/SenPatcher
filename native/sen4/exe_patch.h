#pragma once

#include <cassert>
#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen4 {
enum class GameVersion {
    Jp121,
    En121,
    Jp122,
    En122,
};

inline bool IsGameVersionJp(GameVersion version) {
    return version == GameVersion::Jp121 || version == GameVersion::Jp122;
}

struct PatchExecData {
    HyoutaUtils::Logger* Logger;
    char* TextRegion;
    GameVersion Version;
    char* Codespace;
    char* CodespaceEnd;
};

struct Addresses {
    uint64_t Jp121 = 0;
    uint64_t En121 = 0;
    uint64_t Jp122 = 0;
    uint64_t En122 = 0;
};

inline char* GetCodeAddressJpEn(GameVersion version, char* textRegion, Addresses addresses) {
    uint64_t addr = 0;
    switch (version) {
        case GameVersion::Jp121: addr = addresses.Jp121; break;
        case GameVersion::En121: addr = addresses.En121; break;
        case GameVersion::Jp122: addr = addresses.Jp122; break;
        case GameVersion::En122: addr = addresses.En122; break;
        default: break;
    }
    assert(addr != 0);
    return textRegion + (addr - 0x140001000u);
}

void DeglobalizeMutexes(PatchExecData& execData);
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void AllowSwitchToNightmare(PatchExecData& execData);
void PatchDisableMouseCapture(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchDisablePauseOnFocusLoss(PatchExecData& execData);
void PatchForceSwapConfirmCancel(PatchExecData& execData, bool swapOX);
void PatchFixPcConfirmCancelWhenSwapped(PatchExecData& execData);
void PatchMusicQueueing(PatchExecData& execData);

// fix the crash that happens when opening the Costumes menu
// when you have non-installed DLC costumes equipped
// (the game will just use the base model instead)
void PatchDlcCostumeCrash(PatchExecData& execData);

// changes the number of DLCs the game checks for
void PatchIncreaseDlcCount(PatchExecData& execData, uint32_t newLimit);

// removes the already-used checks for custom DLC. official DLC is unaffected.
void PatchCustomDlcMultiuse(PatchExecData& execData);

} // namespace SenLib::Sen4

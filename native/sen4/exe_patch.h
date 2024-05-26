#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen4 {
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

} // namespace SenLib::Sen4

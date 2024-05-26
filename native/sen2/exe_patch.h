#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen2 {
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
                                uint32_t addressJp,
                                uint32_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x401000u) : (addressEn - 0x401000u));
}

void DeglobalizeMutexes(PatchExecData& execData);
void FixGogGalaxy(PatchExecData& execData);
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void AddCS2ToTitleBar(PatchExecData& execData);
void RemoveTurboAutoSkip(PatchExecData& execData);
void PatchMusicFadeTiming(PatchExecData& execData, uint32_t divisor);
void PatchMusicQueueingOnSoundThreadSide(PatchExecData& execData);
void PatchDisableMouseCapture(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchDisablePauseOnFocusLoss(PatchExecData& execData);
void PatchRemoveDebugLeftovers(PatchExecData& execData);
void PatchLanguageAppropriateVoiceTables(PatchExecData& execData);
void PatchFixControllerMappings(PatchExecData& execData);
void PatchFixArtsSupportCutin(PatchExecData& execData);
void PatchForce0Kerning(PatchExecData& execData);
void PatchAddNullCheckBattleScopeCrashMaybe(PatchExecData& execData);
void PatchForceXInput(PatchExecData& execData);
} // namespace SenLib::Sen2

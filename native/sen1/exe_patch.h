#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen1 {
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
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void PatchTurboMode(PatchExecData& execData,
                    bool removeAutoSkip,
                    int turboModeButton,
                    bool enableR2,
                    bool makeToggle,
                    bool adjustTimersForTurbo);
void PatchThorMasterQuartzString(PatchExecData& execData);
void FixTextboxAdvancePrompt(PatchExecData& execData);
void PatchDisableMouseCapture(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchDisablePauseOnFocusLoss(PatchExecData& execData);
void PatchLanguageAppropriateVoiceTables(PatchExecData& execData);
void PatchFixArtsSupportCutin(PatchExecData& execData);
void PatchForce0Kerning(PatchExecData& execData);
void PatchForceXInput(PatchExecData& execData);
void PatchMusicQueueing(PatchExecData& execData);
void PatchAddCameraSensitivity(PatchExecData& execData, int sensitivity);
} // namespace SenLib::Sen1

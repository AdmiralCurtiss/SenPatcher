#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::TX {
enum class GameVersion {
    Steam,
    Gog,
    Steam_Encrypted,
};

struct PatchExecData {
    HyoutaUtils::Logger* Logger;
    char* TextRegion;
    GameVersion Version;
    char* Codespace;
    char* CodespaceEnd;
};

inline char* GetCodeAddressSteamGog(GameVersion version,
                                    char* textRegion,
                                    uint32_t addressSteam,
                                    uint32_t addressGog) {
    return textRegion
           + (version == GameVersion::Steam ? (addressSteam - 0x401000u)
                                            : (addressGog - 0x401000u));
}

void OverrideLanguage(PatchExecData& execData, bool useJapanese);
void PatchSkipMovies(PatchExecData& execData, bool skipLogos, bool skipAllMovies);
void PatchTurboAndButtonMappings(PatchExecData& execData,
                                 bool makeToggle,
                                 float factor,
                                 bool useJapanese);
void AddSenPatcherVersionToTitle(PatchExecData& execData,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void PatchDisableMouseCamera(PatchExecData& execData);
void PatchShowMouseCursor(PatchExecData& execData);
void PatchEnableBackgroundControllerInput(PatchExecData& execData);
void PatchValidDlcIds(PatchExecData& execData, char* validDlcBitfield, size_t maxDlcId);
void ForceMG04UVs(PatchExecData& execData);
} // namespace SenLib::TX

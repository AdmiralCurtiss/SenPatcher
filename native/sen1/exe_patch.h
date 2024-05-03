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
    Unknown,
    English,
    Japanese,
};

inline char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint32_t addressJp,
                                uint32_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x401000u) : (addressEn - 0x401000u));
}

void DeglobalizeMutexes(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
void AddSenPatcherVersionToTitle(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void RemoveTurboAutoSkip(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd);
void AllowR2NotebookShortcut(HyoutaUtils::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd);
void ChangeTurboModeButton(HyoutaUtils::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           char*& codespace,
                           char* codespaceEnd,
                           int turboModeButton);
void PatchThorMasterQuartzString(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd);
void FixTextboxAdvancePrompt(HyoutaUtils::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd);
void PatchDisableMouseCapture(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd);
void PatchShowMouseCursor(HyoutaUtils::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd);
void PatchDisablePauseOnFocusLoss(HyoutaUtils::Logger& logger,
                                  char* textRegion,
                                  GameVersion version,
                                  char*& codespace,
                                  char* codespaceEnd);
void PatchLanguageAppropriateVoiceTables(HyoutaUtils::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace,
                                         char* codespaceEnd);
void PatchFixArtsSupportCutin(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd);
void PatchForce0Kerning(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
void PatchForceXInput(HyoutaUtils::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd);
void PatchMusicQueueing(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
} // namespace SenLib::Sen1

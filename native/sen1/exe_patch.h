#pragma once

#include <cstdint>

namespace SenPatcher {
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

void DeglobalizeMutexes(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
void AddSenPatcherVersionToTitle(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);
void RemoveTurboAutoSkip(SenPatcher::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd);
void AllowR2NotebookShortcut(SenPatcher::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd);
void ChangeTurboModeButton(SenPatcher::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           char*& codespace,
                           char* codespaceEnd,
                           int turboModeButton);
void PatchThorMasterQuartzString(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd);
void FixTextboxAdvancePrompt(SenPatcher::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd);
void PatchDisableMouseCapture(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd);
void PatchShowMouseCursor(SenPatcher::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd);
void PatchDisablePauseOnFocusLoss(SenPatcher::Logger& logger,
                                  char* textRegion,
                                  GameVersion version,
                                  char*& codespace,
                                  char* codespaceEnd);
void PatchLanguageAppropriateVoiceTables(SenPatcher::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace,
                                         char* codespaceEnd);
void PatchFixArtsSupportCutin(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd);
void PatchForce0Kerning(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
void PatchForceXInput(SenPatcher::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd);
void PatchMusicQueueing(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
} // namespace SenLib::Sen1

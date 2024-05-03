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
void FixGogGalaxy(HyoutaUtils::Logger& logger,
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
void AddCS2ToTitleBar(HyoutaUtils::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd);
void RemoveTurboAutoSkip(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd);
void PatchMusicFadeTiming(HyoutaUtils::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd,
                          uint32_t divisor);
void PatchMusicQueueingOnSoundThreadSide(HyoutaUtils::Logger& logger,
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
void PatchRemoveDebugLeftovers(HyoutaUtils::Logger& logger,
                               char* textRegion,
                               GameVersion version,
                               char*& codespace,
                               char* codespaceEnd);
void PatchLanguageAppropriateVoiceTables(HyoutaUtils::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace,
                                         char* codespaceEnd);
void PatchFixControllerMappings(HyoutaUtils::Logger& logger,
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
void PatchAddNullCheckBattleScopeCrashMaybe(HyoutaUtils::Logger& logger,
                                            char* textRegion,
                                            GameVersion version,
                                            char*& codespace,
                                            char* codespaceEnd);
void PatchForceXInput(HyoutaUtils::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd);
} // namespace SenLib::Sen2

#pragma once

#include <cstdint>

namespace SenPatcher {
struct Logger;
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

void DeglobalizeMutexes(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
void FixGogGalaxy(SenPatcher::Logger& logger,
                  char* textRegion,
                  GameVersion version,
                  char*& codespace,
                  char* codespaceEnd);
void AddSenPatcherVersionToTitle(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd);
void AddCS2ToTitleBar(SenPatcher::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd);
void RemoveTurboAutoSkip(SenPatcher::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd);
void PatchMusicFadeTiming(SenPatcher::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd,
                          uint32_t divisor);
void PatchMusicQueueingOnSoundThreadSide(SenPatcher::Logger& logger,
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
void PatchRemoveDebugLeftovers(SenPatcher::Logger& logger,
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
} // namespace SenLib::Sen2

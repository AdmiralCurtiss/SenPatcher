#pragma once

#include <cstdint>

namespace SenPatcher {
struct Logger;
}

namespace SenLib::Sen4 {
enum class GameVersion {
    Unknown,
    English,
    Japanese,
};

inline char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint64_t addressJp,
                                uint64_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x140001000u)
                                               : (addressEn - 0x140001000u));
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
                                 char* codespaceEnd);

void AllowSwitchToNightmare(SenPatcher::Logger& logger,
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
                                  char*& codespace_,
                                  char* codespaceEnd_);

void PatchForceSwapConfirmCancel(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 bool swapOX);

void PatchFixPcConfirmCancelWhenSwapped(SenPatcher::Logger& logger,
                                        char* textRegion,
                                        GameVersion version,
                                        char*& codespace,
                                        char* codespaceEnd);
} // namespace SenLib::Sen4

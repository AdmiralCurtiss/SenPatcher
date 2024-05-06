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

inline char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint64_t addressJp,
                                uint64_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x140001000u)
                                               : (addressEn - 0x140001000u));
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

void AllowSwitchToNightmare(HyoutaUtils::Logger& logger,
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
                                  char*& codespace_,
                                  char* codespaceEnd_);

void PatchForceSwapConfirmCancel(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 bool swapOX);

void PatchFixPcConfirmCancelWhenSwapped(HyoutaUtils::Logger& logger,
                                        char* textRegion,
                                        GameVersion version,
                                        char*& codespace,
                                        char* codespaceEnd);

void PatchMusicQueueing(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);
} // namespace SenLib::Sen4

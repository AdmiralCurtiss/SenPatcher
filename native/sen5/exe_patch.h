#pragma once

#include <cstdint>

namespace HyoutaUtils {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen5 {
enum class GameVersion {
    English,
};

inline char* GetCodeAddressEn(GameVersion version, char* textRegion, uint64_t addressEn) {
    return textRegion + (addressEn - 0x140001000u);
}

void AddSenPatcherVersionToTitle(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);

void PatchMusicQueueing(HyoutaUtils::Logger& logger,
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

void PatchDisableFpsLimitOnFocusLoss(HyoutaUtils::Logger& logger,
                                     char* textRegion,
                                     GameVersion version,
                                     char*& codespace,
                                     char* codespaceEnd);

// changes the number of DLCs the game checks for
void PatchIncreaseDlcCount(HyoutaUtils::Logger& logger,
                           char* textRegion,
                           GameVersion version,
                           uint32_t newLimit);

} // namespace SenLib::Sen5

#pragma once

#include <cstdint>

namespace SenPatcher {
struct Logger;
}
namespace SenLib::ModLoad {
struct LoadedModsData;
}

namespace SenLib::Sen5 {
enum class GameVersion {
    Unknown,
    English,
};

inline char* GetCodeAddressEn(GameVersion version, char* textRegion, uint64_t addressEn) {
    return textRegion + (addressEn - 0x140001000u);
}

void AddSenPatcherVersionToTitle(SenPatcher::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd,
                                 const SenLib::ModLoad::LoadedModsData& loadedModsData,
                                 bool assetFixCreatingFailed);

void PatchMusicQueueing(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd);

void PatchDisableMouseCapture(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd);

void PatchDisableFpsLimitOnFocusLoss(SenPatcher::Logger& logger,
                                     char* textRegion,
                                     GameVersion version,
                                     char*& codespace,
                                     char* codespaceEnd);

} // namespace SenLib::Sen5

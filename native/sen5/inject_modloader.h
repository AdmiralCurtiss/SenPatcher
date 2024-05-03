#pragma once

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::Sen5 {
enum class GameVersion;

void InjectAtFFileOpen(HyoutaUtils::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder);
void InjectAtBattleScriptExists(HyoutaUtils::Logger& logger,
                                char* textRegion,
                                GameVersion version,
                                char*& codespace,
                                char* codespaceEnd,
                                void* ffileExistsForwarder);
void InjectAtFileExists1(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder);
void InjectAtFileExists2(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder);
void InjectAtFFileGetFilesize(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder);
void InjectAtOpenFSoundFile(HyoutaUtils::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd,
                            void* fsoundOpenForwarder);
} // namespace SenLib::Sen5

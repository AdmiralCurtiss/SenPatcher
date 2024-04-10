#pragma once

namespace SenPatcher {
struct Logger;
}

namespace SenLib::Sen5 {
enum class GameVersion;

void InjectAtFFileOpen(SenPatcher::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder);
void InjectAtBattleScriptExists(SenPatcher::Logger& logger,
                                char* textRegion,
                                GameVersion version,
                                char*& codespace,
                                char* codespaceEnd,
                                void* ffileExistsForwarder);
void InjectAtFileExists1(SenPatcher::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder);
void InjectAtFileExists2(SenPatcher::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder);
void InjectAtFFileGetFilesize(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder);
void InjectAtOpenFSoundFile(SenPatcher::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd,
                            void* fsoundOpenForwarder);
} // namespace SenLib::Sen5

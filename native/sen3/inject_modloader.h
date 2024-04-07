#pragma once

namespace SenPatcher {
struct Logger;
}

namespace SenLib::Sen3 {
enum class GameVersion;

void InjectAtFFileOpen(SenPatcher::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder);
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
} // namespace SenLib::Sen3

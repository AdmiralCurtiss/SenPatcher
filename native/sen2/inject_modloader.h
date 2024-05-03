#pragma once

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::Sen2 {
enum class GameVersion;

void InjectAtFFileOpen(HyoutaUtils::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder);
void InjectAtFFileGetFilesize(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder);
} // namespace SenLib::Sen2

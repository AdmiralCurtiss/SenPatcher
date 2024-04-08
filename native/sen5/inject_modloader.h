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
} // namespace SenLib::Sen5

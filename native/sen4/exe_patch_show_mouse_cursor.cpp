#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchShowMouseCursor(SenPatcher::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* showCursorPos = GetCodeAddressJpEn(version, textRegion, 0x1405fadc4, 0x1405fd344);

    // remove call to ShowCursor(0)
    {
        char* tmp = showCursorPos;
        PageUnprotect page(logger, tmp, 8);
        for (size_t i = 0; i < 7; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
        *tmp++ = static_cast<char>(0xc3); // ret
    }
}
} // namespace SenLib::Sen4
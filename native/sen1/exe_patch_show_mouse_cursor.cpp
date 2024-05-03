#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchShowMouseCursor(HyoutaUtils::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* showCursorPos = GetCodeAddressJpEn(version, textRegion, 0x7be0ea, 0x7bf9ba);

    // remove call to ShowCursor(0)
    {
        char* tmp = showCursorPos;
        PageUnprotect page(logger, tmp, 8);
        for (size_t i = 0; i < 8; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen1

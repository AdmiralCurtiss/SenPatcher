#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchForceXInput(HyoutaUtils::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* position = GetCodeAddressJpEn(version, textRegion, 0x939766, 0x93b036);
    constexpr size_t length = 6;

    {
        char* tmp = position;
        PageUnprotect page(logger, tmp, length);
        for (size_t i = 0; i < length; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen1

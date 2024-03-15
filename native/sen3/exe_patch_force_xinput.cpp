#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchForceXInput(SenPatcher::Logger& logger,
                      char* textRegion,
                      GameVersion version,
                      char*& codespace,
                      char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* xinputCheckPos = GetCodeAddressJpEn(version, textRegion, 0x1406add65, 0x1406ba1b5);

    {
        char* tmp = xinputCheckPos;
        PageUnprotect page(logger, xinputCheckPos, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen3

#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen1 {
void AllowR2NotebookShortcut(HyoutaUtils::Logger& logger,
                             char* textRegion,
                             GameVersion version,
                             char*& codespace,
                             char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* const addressJumpR2NotebookOpen =
        GetCodeAddressJpEn(version, textRegion, 0x5b6fbf, 0x5b812f);
    char* const addressJumpR2NotebookSettings =
        GetCodeAddressJpEn(version, textRegion, 0x6de1a0, 0x6dfaf0);

    {
        char* tmp = addressJumpR2NotebookOpen;
        PageUnprotect page(logger, tmp, 2);
        for (size_t i = 0; i < 2; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
    {
        char* tmp = addressJumpR2NotebookSettings;
        PageUnprotect page(logger, tmp, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen1

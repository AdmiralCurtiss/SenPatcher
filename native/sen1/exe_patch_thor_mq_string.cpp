#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchThorMasterQuartzString(HyoutaUtils::Logger& logger,
                                 char* textRegion,
                                 GameVersion version,
                                 char*& codespace,
                                 char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* mstqrt027 = GetCodeAddressJpEn(version, textRegion, 0xb39d64, 0xb3bfb4) + 6;
    {
        // "mstqrt027" -> "mstqrt27" to fix the texture ID for the HD texture pack
        PageUnprotect page(logger, mstqrt027, 3);
        for (size_t i = 0; i < 3; ++i) {
            (*(mstqrt027 + i)) = (*(mstqrt027 + i + 1));
        }
    }
}
} // namespace SenLib::Sen1

#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchForce0Kerning(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    if (version != GameVersion::English) {
        return;
    }

    using namespace SenPatcher::x86;
    char* addressForce0Kerning = GetCodeAddressJpEn(version, textRegion, 0, 0x5a0631);

    {
        char* tmp = addressForce0Kerning;
        PageUnprotect page(logger, tmp, 2);
        WriteInstruction8(tmp, 0x90); // nop
        WriteInstruction8(tmp, 0x90); // nop
    }
}
} // namespace SenLib::Sen1

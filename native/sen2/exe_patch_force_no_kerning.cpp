#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchForce0Kerning(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    if (version != GameVersion::English) {
        return;
    }

    using namespace SenPatcher::x86;
    char* addressForce0Kerning = GetCodeAddressJpEn(version, textRegion, 0, 0x5328e4);

    {
        char* tmp = addressForce0Kerning;
        PageUnprotect page(logger, tmp, 3);
        WriteInstruction16(tmp, 0x31c0); // xor eax, eax
        WriteInstruction8(tmp, 0x90);    // nop
    }
}
} // namespace SenLib::Sen2

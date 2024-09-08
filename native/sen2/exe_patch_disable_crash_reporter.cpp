#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void DisableCrashReporter(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* call = GetCodeAddressJpEn(version, textRegion, 0x8bb7a5, 0x8bc9c5);

    {
        char* tmp = call;
        PageUnprotect page(logger, tmp, 5);
        WriteInstruction24(tmp, 0xc60100); // mov byte ptr[ecx],0
        WriteInstruction16(tmp, 0x9090);   // nop, nop
    }
}
} // namespace SenLib::Sen2

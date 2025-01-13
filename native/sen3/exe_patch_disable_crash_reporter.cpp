#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void DisableCrashReporter(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* call = GetCodeAddressJpEn(version,
                                    textRegion,
                                    Addresses{.Jp106 = 0x1400572cb,
                                              .En106 = 0x1400572cb,
                                              .Jp107 = 0x1400572cb,
                                              .En107 = 0x1400572cb});

    {
        char* tmp = call;
        PageUnprotect page(logger, tmp, 5);
        WriteInstruction24(tmp, 0xc60100); // mov byte ptr[rcx],0
        WriteInstruction16(tmp, 0x9090);   // nop, nop
    }
}
} // namespace SenLib::Sen3

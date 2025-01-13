#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchForceXInput(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* xinputCheckPos = GetCodeAddressJpEn(version,
                                              textRegion,
                                              Addresses{.Jp106 = 0x1406add65,
                                                        .En106 = 0x1406ba1b5,
                                                        .Jp107 = 0x1406adce5,
                                                        .En107 = 0x1406ba135});

    {
        char* tmp = xinputCheckPos;
        PageUnprotect page(logger, xinputCheckPos, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen3

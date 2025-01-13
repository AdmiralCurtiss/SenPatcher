#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void DeglobalizeMutexes(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* createMutexString1 = GetCodeAddressJpEn(version,
                                                  textRegion,
                                                  Addresses{.Jp121 = 0x1405ae269,
                                                            .En121 = 0x1405b07e9,
                                                            .Jp122 = 0x1405ae1c9,
                                                            .En122 = 0x1405b0749});
    char* createMutexString2 = GetCodeAddressJpEn(version,
                                                  textRegion,
                                                  Addresses{.Jp121 = 0x1400d6051,
                                                            .En121 = 0x1400d60d1,
                                                            .Jp122 = 0x1400d6051,
                                                            .En122 = 0x1400d60d1});
    {
        PageUnprotect page(logger, createMutexString1, 3);
        Emit_XOR_R64_R64(createMutexString1, R64::R8, R64::R8);
    }
    {
        PageUnprotect page(logger, createMutexString2, 7);
        Emit_MOV_R64_IMM64(createMutexString2, R64::R8, 0, 7);
    }
}
} // namespace SenLib::Sen4

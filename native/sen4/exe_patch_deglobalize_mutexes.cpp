#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void DeglobalizeMutexes(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* createMutexString1 = GetCodeAddressJpEn(version, textRegion, 0x1405ae269, 0x1405b07e9);
    char* createMutexString2 = GetCodeAddressJpEn(version, textRegion, 0x1400d6051, 0x1400d60d1);
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

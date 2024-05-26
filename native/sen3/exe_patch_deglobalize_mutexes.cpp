#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void DeglobalizeMutexes(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* createMutexString1 = GetCodeAddressJpEn(version, textRegion, 0x14054fb09, 0x14055bc99);
    char* createMutexString2 = GetCodeAddressJpEn(version, textRegion, 0x14011c611, 0x14011cb11);
    {
        PageUnprotect page(logger, createMutexString1, 3);
        Emit_XOR_R64_R64(createMutexString1, R64::R8, R64::R8);
    }
    {
        PageUnprotect page(logger, createMutexString2, 7);
        Emit_MOV_R64_IMM64(createMutexString2, R64::R8, 0, 7);
    }
}
} // namespace SenLib::Sen3

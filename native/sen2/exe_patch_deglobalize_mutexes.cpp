#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void DeglobalizeMutexes(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* createMutexString = GetCodeAddressJpEn(version, textRegion, 0x71d484, 0x71e4e4);
    {
        // call CreateMutex without names so multiple instances of CS2 don't block eachother
        PageUnprotect page(logger, createMutexString, 3);
        WriteInstruction24(createMutexString, 0x6a0090); // push 0, nop
    }
}
} // namespace SenLib::Sen2

#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void DeglobalizeMutexes(HyoutaUtils::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* createMutexString = GetCodeAddressJpEn(version, textRegion, 0x7cf9c3, 0x7d1293);
    {
        // call CreateMutex without names so multiple instances of CS2 don't block eachother
        PageUnprotect page(logger, createMutexString, 3);
        WriteInstruction24(createMutexString, 0x31c090); // xor eax,eax, nop
    }
}
} // namespace SenLib::Sen1

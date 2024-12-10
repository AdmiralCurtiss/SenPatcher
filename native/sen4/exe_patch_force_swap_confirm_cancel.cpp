#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchForceSwapConfirmCancel(PatchExecData& execData, bool swapOX) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* isSwitchButtonFuncPos = GetCodeAddressJpEn(version, textRegion, 0x1405fa710, 0x1405fcc90);

    // replace function that usually checks for (mapping == Switch) with (SwapOX && mapping != PC)
    {
        char* inject = isSwitchButtonFuncPos;
        PageUnprotect page(logger, inject, 0x10);
        for (size_t i = 0; i < 0x10; ++i) {
            inject[i] = static_cast<char>(0xcc);
        }

        if (swapOX) {
            // always return 1, except when PC mappings are active return 0
            WriteInstruction32(inject, 0x83794002); // cmp   dword ptr [rcx+40h],2
            WriteInstruction24(inject, 0x0f95c0);   // setne al
            Emit_RET(inject);
        } else {
            // trivial, always just return 0
            Emit_XOR_R64_R64(inject, R64::RAX, R64::RAX);
            Emit_RET(inject);
        }
    }
}
} // namespace SenLib::Sen4

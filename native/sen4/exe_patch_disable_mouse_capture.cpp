#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchDisableMouseCapture(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* captureMouseCursorPos = GetCodeAddressJpEn(version, textRegion, 0x14030ab6f, 0x14030d62f);
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version, textRegion, 0x1400b1e78, 0x1400b1e98);
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version, textRegion, 0x1400b2034, 0x1400b2054);
    char* processMouseFuncPos = GetCodeAddressJpEn(version, textRegion, 0x1400b2cd1, 0x1400b2cf1);

    // change functions that capture the mouse cursor to not do that
    {
        PageUnprotect page(logger, captureMouseCursorPos, 1);
        *captureMouseCursorPos = static_cast<char>(0xeb); // jz -> jmp
    }

    // change function that handles camera movement to not react to mouse movement
    // and not to fall back to WASD camera movement either (legacy code...?)
    {
        PageUnprotect page(logger, cameraMouseFuncPos1, 6);
        BranchHelper4Byte branch;
        branch.SetTarget(cameraMouseFuncPos2);
        char* tmp = cameraMouseFuncPos1;
        branch.WriteJump(tmp, JumpCondition::JMP);
        *tmp = static_cast<char>(0x90); // nop
    }

    // skip mouse movement processing function or something like that?
    {
        char* tmp = processMouseFuncPos;
        PageUnprotect page(logger, tmp, 5);
        for (size_t i = 0; i < 5; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen4

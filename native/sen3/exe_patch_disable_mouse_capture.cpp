#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDisableMouseCapture(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    using namespace SenPatcher::x64;
    char* captureMouseCursorPos1 =
        GetCodeAddressJpEn(version, textRegion, 0x140320db8, 0x14032a2c8);
    char* captureMouseCursorPos2 =
        GetCodeAddressJpEn(version, textRegion, 0x140320f85, 0x14032a495);
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version, textRegion, 0x1400f7be8, 0x1400f8268);
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version, textRegion, 0x1400f7d61, 0x1400f83e1);
    char* processMouseFuncPos = GetCodeAddressJpEn(version, textRegion, 0x1400f88bd, 0x1400f8f3d);

    // change functions that capture the mouse cursor to not do that
    {
        PageUnprotect page(logger, captureMouseCursorPos1, 1);
        *captureMouseCursorPos1 = 0xeb; // jz -> jmp
    }
    {
        PageUnprotect page(logger, captureMouseCursorPos2, 1);
        *captureMouseCursorPos2 = 0xeb; // jz -> jmp
    }

    // change function that handles camera movement to not react to mouse movement
    // and not to fall back to WASD camera movement either (legacy code...?)
    {
        PageUnprotect page(logger, cameraMouseFuncPos1, 6);
        BranchHelper4Byte branch;
        branch.SetTarget(cameraMouseFuncPos2);
        char* tmp = cameraMouseFuncPos1;
        branch.WriteJump(tmp, JumpCondition::JMP);
        *tmp = 0x90; // nop
    }

    // skip mouse movement processing function or something like that?
    {
        char* tmp = processMouseFuncPos;
        PageUnprotect page(logger, tmp, 5);
        for (size_t i = 0; i < 5; ++i) {
            *tmp++ = 0x90; // nop
        }
    }
}
} // namespace SenLib::Sen3

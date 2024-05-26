#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchDisableMouseCapture(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* captureMouseCursorPos = GetCodeAddressJpEn(version, textRegion, 0x581b0e, 0x581ece);
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version, textRegion, 0x4148b8, 0x4148c8);
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version, textRegion, 0x414a73, 0x414a83);
    char* processMouseFuncPos = GetCodeAddressJpEn(version, textRegion, 0x4154d2, 0x4154e2);

    // change function that captures the mouse cursor to not do that
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
} // namespace SenLib::Sen2

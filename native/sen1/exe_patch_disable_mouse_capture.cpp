#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void PatchDisableMouseCapture(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd) {
    using namespace SenPatcher::x86;
    char* captureMouseCursorPos = GetCodeAddressJpEn(version, textRegion, 0x5de6c6, 0x5df536);
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version, textRegion, 0x4464e5, 0x446635);
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version, textRegion, 0x44667a, 0x4467ca);
    char* processMouseFuncPos = GetCodeAddressJpEn(version, textRegion, 0x446e3a, 0x446f8a);

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

    // there's a third function at 0x53c766 that seems involved here,
    // but leaving it alone seems to work just fine...

    // skip mouse movement processing function or something like that?
    {
        char* tmp = processMouseFuncPos;
        PageUnprotect page(logger, tmp, 5);
        for (size_t i = 0; i < 5; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen1

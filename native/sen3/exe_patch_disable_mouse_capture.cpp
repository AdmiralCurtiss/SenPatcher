#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDisableMouseCapture(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* captureMouseCursorPos1 = GetCodeAddressJpEn(version,
                                                      textRegion,
                                                      Addresses{.Jp106 = 0x140320db8,
                                                                .En106 = 0x14032a2c8,
                                                                .Jp107 = 0x140320d38,
                                                                .En107 = 0x14032a248});
    char* captureMouseCursorPos2 = GetCodeAddressJpEn(version,
                                                      textRegion,
                                                      Addresses{.Jp106 = 0x140320f85,
                                                                .En106 = 0x14032a495,
                                                                .Jp107 = 0x140320f05,
                                                                .En107 = 0x14032a415});
    char* cameraMouseFuncPos1 = GetCodeAddressJpEn(version,
                                                   textRegion,
                                                   Addresses{.Jp106 = 0x1400f7be8,
                                                             .En106 = 0x1400f8268,
                                                             .Jp107 = 0x1400f7be8,
                                                             .En107 = 0x1400f8268});
    char* cameraMouseFuncPos2 = GetCodeAddressJpEn(version,
                                                   textRegion,
                                                   Addresses{.Jp106 = 0x1400f7d61,
                                                             .En106 = 0x1400f83e1,
                                                             .Jp107 = 0x1400f7d61,
                                                             .En107 = 0x1400f83e1});
    char* processMouseFuncPos = GetCodeAddressJpEn(version,
                                                   textRegion,
                                                   Addresses{.Jp106 = 0x1400f88bd,
                                                             .En106 = 0x1400f8f3d,
                                                             .Jp107 = 0x1400f88bd,
                                                             .En107 = 0x1400f8f3d});

    // change functions that capture the mouse cursor to not do that
    {
        PageUnprotect page(logger, captureMouseCursorPos1, 1);
        *captureMouseCursorPos1 = static_cast<char>(0xeb); // jz -> jmp
    }
    {
        PageUnprotect page(logger, captureMouseCursorPos2, 1);
        *captureMouseCursorPos2 = static_cast<char>(0xeb); // jz -> jmp
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
} // namespace SenLib::Sen3

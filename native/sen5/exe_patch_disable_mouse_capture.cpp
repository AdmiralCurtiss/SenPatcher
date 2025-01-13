#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchDisableMouseCapture(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* captureMouseCursorPos = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140838e7b, .En115 = 0x1408303fb});
    char* mouseMovementProcessingPos = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140880e05, .En115 = 0x1408783f5});

    // remove call to ClipCursor()
    {
        char* tmp = captureMouseCursorPos;
        PageUnprotect page(logger, tmp, 6);
        for (size_t i = 0; i < 6; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }

    // nop the += of the mouse position from the RAWMOUSE struct
    {
        char* tmp = mouseMovementProcessingPos;
        PageUnprotect page(logger, tmp, 12);
        for (size_t i = 0; i < 12; ++i) {
            *tmp++ = static_cast<char>(0x90); // nop
        }
    }
}
} // namespace SenLib::Sen5

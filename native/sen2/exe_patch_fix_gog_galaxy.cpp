#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
static void WriteByte(HyoutaUtils::Logger& logger, char* ptr, int byte) {
    SenPatcher::x86::PageUnprotect page(logger, ptr, 1);
    *ptr = static_cast<char>(byte);
}

void FixGogGalaxy(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;

    // 1.4.1/2 was seemingly compiled with the wrong gog galaxy headers, resulting in a few
    // incorrect function offsets. amazingly this doesn't crash, but it does mean achievements
    // don't work, so fix that
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6adb69, 0x6aeb99), 0x40);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6adbd9, 0x6aec09), 0x40);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6ae019, 0x6af049), 0x40);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6ae0a9, 0x6af109), 0x40);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6ae5f5, 0x6af645), 0x2c);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6ae756, 0x6af7a6), 0x2c);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6af217, 0x6b0267), 0x48);
    WriteByte(logger, GetCodeAddressJpEn(version, textRegion, 0x6af37f, 0x6b03cf), 0x2c);
}
} // namespace SenLib::Sen2

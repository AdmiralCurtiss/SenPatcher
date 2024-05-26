#include "exe_patch.h"

#include <bit>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

#include "senpatcher_version.h"

namespace SenLib::Sen2 {
void AddCS2ToTitleBar(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;
    char* titleBarStringData = GetCodeAddressJpEn(version, textRegion, 0x6af3ec, 0x6b043c);
    PageUnprotect page(logger, titleBarStringData, 6);

    // the title bar just says Cold Steel without the II, add it

    // update string length
    char oldLength = *titleBarStringData;
    *titleBarStringData = static_cast<char>(oldLength + 3);

    uint32_t addressTitleString;
    std::memcpy(&addressTitleString, titleBarStringData + 2, 4);

    uint32_t addressNewTitleString = std::bit_cast<uint32_t>(codespace);
    {
        const char* tmp = std::bit_cast<const char*>(addressTitleString);
        for (size_t i = 0; i < 44; ++i) {
            *codespace = *tmp;
            ++codespace;
            ++tmp;
        }
        *codespace++ = 'I';
        *codespace++ = 'I';
        *codespace++ = ' ';
        while (*tmp != 0) {
            *codespace = *tmp;
            ++codespace;
            ++tmp;
        }
    }

    std::memcpy(titleBarStringData + 2, &addressNewTitleString, 4);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen2

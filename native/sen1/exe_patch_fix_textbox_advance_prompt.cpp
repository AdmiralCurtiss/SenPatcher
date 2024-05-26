#include "exe_patch.h"

#include <algorithm>
#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
void FixTextboxAdvancePrompt(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    if (version != GameVersion::Japanese) {
        return;
    }

    // this inits the struct responsible for telling which button to use for the textbox text
    // advance prompts. the JP version incorrectly uses the cancel button instead of the confirm
    // button, so swap that around
    {
        char* const tmp = GetCodeAddressJpEn(version, textRegion, 0x467222, 0);
        PageUnprotect page(logger, tmp, 1);
        *tmp = 4;
    }
}
} // namespace SenLib::Sen1

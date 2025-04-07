#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1310[] = "Text fixes in school clubhouse.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1310 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1310.dat",
            207169,
            HyoutaUtils::Hash::SHA1FromHexString("2260799e8a2f431f6bf85af27c98fb3cef197841"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // linebreaks
        // "Hmph. Who cares about the Autumn\x01Festival, anyway? Everyone's all excited\x01over
        // nothing."
        std::swap(bin[0xea21], bin[0xea29]);
        // "#E[7]#M_0And now that you have a national tour\x01under your belts, it's time you guys
        // stop\x01acting like mewling newbies."
        std::swap(bin[0x2c545], bin[0x2c54a]);
        // "#E[1]#M[9](Probably shouldn't bug them\x01right now. I'll just leave, I guess.)"
        std::swap(bin[0x2c919], bin[0x2c91f]);

        // "#4KHuh. Heard you guys were in the area,\x01but I didn't expect you to be HERE."
        // "#4KWhat're you guys doing in the dojo,\x01of all places?"
        // These are written as if they were part of the same conversation, but they're actually
        // mutually exclusive lines depending on if you've heard about Spika being at school before
        // or not. The first one is 'have heard' and the second one is 'have not heard'.
        // These are pretty good as-is, but the follow-up line from Wakaba has 'it' referring to the
        // dojo, so it needs to be mentioned explicitly in the first variant.
        // See also a similar pair of lines in t1210.
        std::swap(bin[0x2c03e], bin[0x2c042]);
        patcher.ReplacePartialCommand(0x2c013, 0x51, 0x2c05d, 4, STR_SPAN("in the dojo"));
        std::swap(bin[0x2c096], bin[0x2c0a3]);
        patcher.ExtendPartialCommand(0x2c07a, 0x3a, 0x2c096, STR_SPAN(" HERE"));

        // "#E[C]#M_0Speaking of which, I think a student\x01posted a request on NiAR,too."
        // missing space
        // (Chapter 2, quest tutorial)
        patcher.ExtendPartialCommand(0x2d9ec, 0x10d, 0x2DAA3, STR_SPAN(" "));


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1310

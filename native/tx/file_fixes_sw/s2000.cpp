#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s2000 {
std::string_view GetDescription() {
    return "Text fixes in Memorial Park.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s2000.dat",
            513601,
            HyoutaUtils::Hash::SHA1FromHexString("858697a0cb8af674b4fdf070f3fc17c2068a3a08"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // while we're here, better align these so they're not obviously off-center to the right
        bin[0x483d3] = 0x01;
        bin[0x4841a] = 0x01;
        std::swap(bin[0x48436], bin[0x48439]);
        bin[0x48480] = 0x01;

        // "...Sora decided to do some personal training with Chiaki."
        // -> "...I decided to do some personal training with Chiaki."
        // the line is voice acted, seems weird otherwise
        patcher.ReplacePartialCommand(0x48454, 0x44, 0x48460, 0x4, {{0x49}});

        // Expert -> Advanced
        // to match what the skateboard levels are actually called in the minigame
        patcher.ReplacePartialCommand(0x7218e, 0x23, 0x721a9, 6, STR_SPAN("Advanced"));
        patcher.ReplacePartialCommand(0x72204, 0x23, 0x7221f, 6, STR_SPAN("Advanced"));

        // "I don't carry my Xiphone around when I'm conducting fieldwork, o I tend to lose track of
        // time."
        // missing 's'
        patcher.ExtendPartialCommand(0x19325, 0xa4, 0x193a8, {{0x73}});

        // "(This guy is still at it... It's already night. Isn't he cold?)"
        // this is just wrong, it's still the middle of the day. the old script was
        // "Even though It's already pretty cold at night."
        // which, capitalization aside, appears to be more accurate, so...
        patcher.ReplacePartialCommand(0x193ed,
                                      0x46,
                                      0x1940e,
                                      0x22,
                                      STR_SPAN("Even though\x01"
                                               "it's already pretty cold at night."));

        // "#2P#0T*gulp*#20W #1000W*gulp*#25W...#1000W"
        // patcher.ReplacePartialCommand(0x49276, 0x30, 0x4929d, 0x6, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2000

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s1000[] = "Text fixes in Kokonoe Shrine (outside).";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s1000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s1000.dat",
            137113,
            HyoutaUtils::Hash::SHA1FromHexString("b9c1b1a7d7b6a860f08316058b11c24231825783"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P...!#10W\x01#1000W#5SWhat is THAT?!"
        // harmless
        // (EV_08_00_01)
        // patcher.ReplacePartialCommand(0x13609, 0x2d, 0x13623, 0x11, "");

        // "I'll protect, Grandma!"
        // add a 'you'
        // (before entering Pandora, small kid in the corner)
        patcher.ExtendPartialCommand(0xdc79, 0x63, 0xDCD0, STR_SPAN(" you"));

        // "(＂I hope I can score a date with the\x01girl standing next to me right now…＂)"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0xbeb6, 0x55, 0xbeb9, 0x50, "");
        bin[0xBF02] = '.';
        bin[0xBF03] = '.';
        bin[0xBF04] = '.';

        // "#K#0TI see. That's the key, isn't it?"
        // "the key" -> "a key"
        // (Epilogue, the key from Sanae)
        patcher.ReplacePartialCommand(0x1bc8f, 0x8a, 0x1BCA5, 3, {{'a'}});

        // "Uh. Um, well... Do you have any\x01free time this weekend, Sanae?"
        // "Hmm? Not really. I didn't\x01have anything planned."
        // "R-Really?! Wh-What a coincidence!\x01I don't have any plans this weekend,\x01either!"
        // this doesn't work, you can't answer 'not really' to 'do you have free time' if you mean
        // 'yes i have free time'
        // we're just gonna do "Not really." -> "I suppose."
        // (Epilogue, the guy standing next to Sanae)
        patcher.ReplacePartialCommand(0x8c1b, 0x35, 0x8C23, 10, STR_SPAN("I suppose"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s1000

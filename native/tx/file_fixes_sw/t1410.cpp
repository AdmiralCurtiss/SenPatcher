#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1410[] = "Text fixes in school pool.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1410 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1410.dat",
            138137,
            HyoutaUtils::Hash::SHA1FromHexString("2656b1d6b6f6393edb068a6761e67f2060700431"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // memories menu inconsistencies for 2nd swimming scene (MM_07_01_02)

        // "#6S#0T#2000WLook at him go...! ㈱"
        // in-game version doesn't have the '...'
        patcher.RemovePartialCommand(0x1654a, 0x27, 0x16567, 3);

        // "#E[9]#M_0Hmm. Rion and Hiiragi go without saying, but Shiori looks great in a swimsuit,
        // too."
        // linebreak is missing
        // patcher.ReplacePartialCommand(0x16bd1, 0x135, 0x16c1d, 0x57, "");
        bin[0x16C49] = 0x1;

        // "#E[EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE5]#M_0My childhood friend's
        // got,\x01y'know...NOTHIN' on them.\x01I think it works for Towa, though."
        // linebreaks are different
        bin[0x16CC7] = ' ';
        bin[0x16CE1] = ' ';
        bin[0x16CF6] = 0x1;
        patcher.ExtendPartialCommand(0x16bd1, 0x135, 0x16CD1, {{'\x01'}});

        // "#K#0T#FHey. Stop staring."
        // -> "Dude, stop staring."
        patcher.ReplacePartialCommand(0x16d22, 0x23, 0x16D31, 6, STR_SPAN("Dude, s"));

        // "#1P#5SSo... Let's race! You and me, one-on-one!"
        // -> "So...let's"
        patcher.ReplacePartialCommand(0x17085, 0x39, 0x17098, 2, STR_SPAN("l"));

        // "#E[9]#M_0There were a ton of people who\x01helped make that concert shine.\x01I really
        // can't thank all of them enough."
        // linebreaks are different
        // patcher.ReplacePartialCommand(0x182ec, 0x10b, 0x1838a, 0x6b, "");
        std::swap(bin[0x183AC], bin[0x183B3]);
        std::swap(bin[0x183CC], bin[0x183DB]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1410

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9870[] = "Text fixes in Sky Tomb Boundary (boss).";
}

namespace SenLib::TX::FileFixesSw::m9870 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9870.dat",
            104313,
            HyoutaUtils::Hash::SHA1FromHexString("d54ebf4905c43d6c76a3dc46a7674aeb6a637c4a"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#3C#1P#3CAnd what're you two doing here,\x01huh? Shiori and Towa, I mean."
        // no wait in English text
        // (EV_17_27_01)
        // patcher.ReplacePartialCommand(0x6b84, 0x50, 0x6b92, 0x40, "");

        // "#E[4]#M_4#3C#2P#3CAnd there are so, so many fond memories\x01to reminisce. #7W#1000WJust
        // like with Sousuke Kokonoe\x01and everyone else ten years ago."
        // harmless
        // (EV_17_27_01)
        // patcher.ReplacePartialCommand(0x782d, 0x9a, 0x7880, 0x45, "");

        // "...Isn't that right#6W? #800W#300WTwilight Apostle?"
        // This one is already timed perfectly. Nice!
        // (EV_17_27_01)
        // patcher.ReplacePartialCommand(0x805d, 0x3d, 0x8082, 0x16, "");

        // "#1P#6SYou asked for it! #15W#1000WLet's go, guys!"
        // harmless
        // (EV_17_27_01)
        // patcher.ReplacePartialCommand(0x1248c, 0x3b, 0x124b0, 0x15, "");

        // "#E[U]#M_0#3C#250W#3C...But...I just...#14W#250Wdon't...understand..."
        // harmless
        // (EV_17_27_02)
        // patcher.ReplacePartialCommand(0x1510d, 0x4e, 0x1513f, 0x1a, "");

        // "#3C#250W#1P#3CTen years ago...#16W Wh-#250WWhy...#22W #250WWhy#16W...#250Wwas I
        // born...?"
        // oof... that's a lot of delay commands. we'll drop all of them except the first.
        // (EV_17_27_02)
        {
            static constexpr size_t offset = 0x151f1;
            static constexpr size_t size = 0x62;

            std::vector<char> tmp;
            tmp.reserve(size);
            tmp.insert(tmp.end(), &bin[offset], &bin[offset + 0x26]);
            bool drop = false;
            for (size_t i = 0x26; i < size; ++i) {
                const char c = bin[offset + i];
                if (c == '#') {
                    drop = true;
                }
                if (!drop) {
                    tmp.push_back(c);
                }
                if (drop && c == 'W') {
                    drop = false;
                }
            }
            patcher.ReplaceCommand(offset, size, tmp);
        }


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9870

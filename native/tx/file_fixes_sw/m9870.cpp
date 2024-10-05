#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::m9870 {
std::string_view GetDescription() {
    return "Text fixes in Sky Tomb Boundary (boss).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9870.dat",
            104313,
            HyoutaUtils::Hash::SHA1FromHexString("42eaa838262cc5821973f1633828f2bb1ec8084c"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#3C#1P#3CAnd what're you two doing here,\x01huh? Shiori and Towa, I mean."
        // patcher.ReplacePartialCommand(0x6b83, 0x50, 0x6b91, 0x40, "");

        // "#E[4]#M_4#3C#2P#3CAnd there are so, so many fond memories\x01to reminisce. #7W#1000WJust
        // like with Sousuke Kokonoe\x01and everyone else ten years ago."
        // patcher.ReplacePartialCommand(0x782c, 0x9a, 0x787f, 0x45, "");

        // "...Isn't that right#6W? #800W#300WTwilight Apostle?"
        // patcher.ReplacePartialCommand(0x805c, 0x3d, 0x8081, 0x16, "");

        // "#1P#6SYou asked for it! #15W#1000WLet's go, guys!"
        // patcher.ReplacePartialCommand(0x12486, 0x3b, 0x124aa, 0x15, "");

        // "#E[U]#M_0#3C#250W#3C...But...I just...#14W#250Wdon't...understand..."
        // patcher.ReplacePartialCommand(0x15109, 0x4e, 0x1513b, 0x1a, "");

        // "#3C#250W#1P#3CTen years ago...#16W Wh-#250WWhy...#22W #250WWhy#16W...#250Wwas I
        // born...?"
        // patcher.ReplacePartialCommand(0x151ed, 0x62, 0x1523a, 0x13, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9870

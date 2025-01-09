#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1000[] = "Text fixes outside school (including roof).";
}

namespace SenLib::TX::FileFixesSw::t1000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1000.dat",
            335873,
            HyoutaUtils::Hash::SHA1FromHexString("2a3eb965a665ebdf1983a114f1e9b883b1040e15"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#0T#5S#800WAnnihilate!#20W\x01#7S#1000WAkashic Blade!"
        // patcher.ReplacePartialCommand(0x1c943, 0x3d, 0x1c969, 0x14, "");

        // "#E[9999999999999999999999999999999998]#M_0#800WBut if I do...then tomorrow will
        // come.\x01If I stay up, I can delay it...even a little."
        // patcher.ReplacePartialCommand(0x23a2d, 0x12c, 0x23a96, 0x59, "");

        // "#E[999999999999999999999999999999FFFFFFFFF9]#M_A#600W...But here in Japan... And with a
        // boy...\x01#10W#3S#600WIt feels...different."
        // patcher.ReplacePartialCommand(0x24bef, 0x8a, 0x24c5d, 0x1a, "");

        // "#0T#400WHaha...#10W\x01#6S#1200WHahahahaha!"
        // patcher.ReplacePartialCommand(0x24d4a, 0x33, 0x24d69, 0x11, "");

        // "#500WYeah. Let's, #7W#500WAsuka."
        // patcher.ReplacePartialCommand(0x2552e, 0x2a, 0x2554b, 0xb, "");

        // "#3K#F#1000WShoot, you're right!\x01#1000W#5SWe gotta book it!"
        // patcher.ReplacePartialCommand(0x305c1, 0x44, 0x305ef, 0x14, "");

        // "#500WYeah, yeah. #1000WAnd wait a second!\x01#1000W#5SI told you not to call me that!"
        // patcher.ReplacePartialCommand(0x30673, 0x5c, 0x306ab, 0x22, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1000

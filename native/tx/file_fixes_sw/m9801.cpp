#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9801[] = "Text fixes in Boundary of Blood (boss room).";
}

namespace SenLib::TX::FileFixesSw::m9801 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9801.dat",
            22609,
            HyoutaUtils::Hash::SHA1FromHexString("e9c91781beb869b014dc49a37d7d88652e90be47"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1POn the count of three!{n}#18W...#1000W#5S1#29W, #1000W2...!"
        // for once, the text delay thing *almost* works, but it's slightly off with the VO it's
        // supposed to match. so let's try to fix that...
        // patcher.ReplacePartialCommand(0x2056, 0x47, 0x208f, 0xb, "");

        // "#1PKou#20W! #1000WW-Wait, #5SWHAT?!"
        // patcher.ReplacePartialCommand(0x2704, 0x2d, 0x2726, 0x9, "");

        // "#3K#FYeah#19W!#1000W\x01#5SLet's get this over with!"
        // patcher.ReplacePartialCommand(0x2b24, 0x3b, 0x2b41, 0x1c, "");

        // "#1P#3C#800W#3CYou got them back."
        // patcher.ReplacePartialCommand(0x46d3, 0x2a, 0x46e6, 0x15, "");

        // "#1P#3C#600W#3CI might need to spirit\x01some others next time."
        // patcher.ReplacePartialCommand(0x472d, 0x45, 0x4740, 0x30, "");

        // "#K#0T#F...!\x01#1000W#5SSTOP!"
        // patcher.ReplacePartialCommand(0x486c, 0x20, 0x4881, 0x8, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9801

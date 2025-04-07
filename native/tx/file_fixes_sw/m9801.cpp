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
            22625,
            HyoutaUtils::Hash::SHA1FromHexString("0f56a2a4eca46a6b2e4e441729788309b2254e8e"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1POn the count of three!\x01#18W...#1000W#5S1#29W, #1000W2...!"
        // for once, the text delay thing *almost* works, but it's slightly off with the VO it's
        // supposed to match. so let's try to fix that...
        // patcher.ReplacePartialCommand(0x2062, 0x47, 0x209b, 0xb, "");

        // "#1PKou#20W! #1000WW-Wait...\x01#5SWHAT?!"
        // patcher.ReplacePartialCommand(0x2710, 0x2f, 0x2734, 0x9, "");

        // "#3K#FYeah#19W!#1000W\x01#5SLet's get this over with!"
        // patcher.ReplacePartialCommand(0x2b32, 0x3b, 0x2b4f, 0x1c, "");

        // "#1P#3C#800W#3CYou got them back."
        // patcher.ReplacePartialCommand(0x46df, 0x2a, 0x46f2, 0x15, "");

        // "#1P#3C#600W#3CI might need to spirit\x01away some others next time."
        // patcher.ReplacePartialCommand(0x4739, 0x4a, 0x474c, 0x35, "");

        // "#K#0T#F...!\x01#1000W#5SSTOP!"
        // patcher.ReplacePartialCommand(0x487d, 0x20, 0x4892, 0x8, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9801

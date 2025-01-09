#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3000[] = "Text fixes outside Acros Tower.";
}

namespace SenLib::TX::FileFixesSw::s3000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3000.dat",
            67153,
            HyoutaUtils::Hash::SHA1FromHexString("79d949836828301f301d102fe3b3533416d65fcf"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#7C#800W#7CLikewise...#1000WMr. Gorou."
        // patcher.ReplacePartialCommand(0x4788, 0x31, 0x47a6, 0x10, "");

        // "There you are!\x01#1000WJust give it up already!\x01We're gonna catch you sooner or-"
        // patcher.ReplacePartialCommand(0x6abd, 0x5b, 0x6ad4, 0x41, "");

        // "#0T#300W#5S#3C#3CCome here...\x01#300WCome here..."
        // patcher.ReplacePartialCommand(0x7081, 0x39, 0x70a7, 0x11, "");

        // "#100W#5SAfter all this time... Damn it!"
        // patcher.RemovePartialCommand(0x73c5, 0x40, 0x73dc, 0x5);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3000

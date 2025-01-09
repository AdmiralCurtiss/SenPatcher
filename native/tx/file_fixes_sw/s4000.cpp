#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s4000[] = "Text fixes in Residental Area.";
}

namespace SenLib::TX::FileFixesSw::s4000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s4000.dat",
            93633,
            HyoutaUtils::Hash::SHA1FromHexString("9d59d2c2263a4fb451be08135679192920be9b9f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2P#5S#800WGuys#50W... #800WPlease lend me your\x01strength one last time."
        // patcher.ReplacePartialCommand(0x7629, 0x51, 0x7648, 0x30, "");

        // "#4S#800WYeah.\x01#40W#1000W#5SLet's go!"
        // patcher.ReplacePartialCommand(0x833c, 0x2e, 0x835c, 0xc, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s4000

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9810[] = "Text fixes in " DUNGEON_NAME_m9810 ".";
}

namespace SenLib::TX::FileFixesSw::m9810 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9810.dat",
            33257,
            HyoutaUtils::Hash::SHA1FromHexString("6db5233e94d3ee1e23849dec61ce238c259a4ea2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[M]#M_B#e[B]#5S#0T#FBack off#7W, #1000W#6SASSHOLE!"
        // again, this *almost* works!
        // patcher.ReplacePartialCommand(0x4b47, 0x3f, 0x4b78, 0xb, "");

        // "#K#F...! #1000W#5SI won't back down either!"
        // patcher.ReplacePartialCommand(0x52be, 0x35, 0x52d5, 0x1c, "");

        // "#2PWe will surely win with everyone here!\x01#8W#1000W#5SCome on!"
        // patcher.ReplacePartialCommand(0x5de9, 0x48, 0x5e24, 0xb, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9810

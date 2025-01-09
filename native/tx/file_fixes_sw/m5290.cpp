#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m5290[] = "Text fixes in Birdcage Corridor (boss room).";
}

namespace SenLib::TX::FileFixesSw::m5290 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m5290.dat",
            23409,
            HyoutaUtils::Hash::SHA1FromHexString("87b91fb4c98ffeba6d3372e1df7a5c7604454c77"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#3K#F#5S#0T#800WLet's go...\x01Ecthelion Hearts!"
        // patcher.ReplacePartialCommand(0x3e68, 0x38, 0x3e7b, 0x22, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m5290

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t1001 {
std::string_view GetDescription() {
    return "Text fixes outside school (Mist Castle).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1001.dat",
            18113,
            HyoutaUtils::Hash::SHA1FromHexString("0496f7fc633ca9629789c4f076adbca11c8b164d"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#800WI can't get in contact with Shiori#15W,\x01#800WRyouta, Jun, or
        // Towa.\x01#10W#5S#800WDamn it!"
        // patcher.ReplacePartialCommand(0x2369, 0x66, 0x23c0, 0xd, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1001

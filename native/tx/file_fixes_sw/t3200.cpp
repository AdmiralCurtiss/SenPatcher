#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t3200 {
std::string_view GetDescription() {
    return "Text fixes in Yanagi Sports.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3200.dat",
            104225,
            HyoutaUtils::Hash::SHA1FromHexString("d6f92eb40476cdd6a003c045c5827fc49cab3557"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Kou, Shio, and Sora.."
        // add another period for ellipsis
        patcher.ExtendPartialCommand(0x7eb5, 0x1a, 0x7ecb, {{0x2e}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3200

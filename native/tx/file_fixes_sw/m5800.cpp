#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::m5800 {
std::string_view GetDescription() {
    return "Text fixes in Ruins of Poisonous Filth.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m5800.dat",
            13641,
            HyoutaUtils::Hash::SHA1FromHexString("10f9c0ced07bdc181bab13f127147e32d89a69c6"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P#5SLet's do this!#1000W\x01#5SYou got my back, Hokuto?!"
        // patcher.ReplacePartialCommand(0x1c78, 0x3c, 0x1c96, 0x1c, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m5800

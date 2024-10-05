#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::m5801 {
std::string_view GetDescription() {
    return "Text fixes in Ruins of Poisonous Filth (boss room).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m5801.dat",
            16153,
            HyoutaUtils::Hash::SHA1FromHexString("d925f2f1689fd7c2b0e28e4ef9f6d6dc1102df12"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#3K#FYou can count on me...\x01#17W#1000W#5SLet's do this, Hokuto!"
        // patcher.ReplacePartialCommand(0x2eac, 0x49, 0x2eda, 0x19, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m5801

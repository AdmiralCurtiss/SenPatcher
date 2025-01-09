#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m2190[] = "Text fixes in Moonlit Garden (boss room).";
}

namespace SenLib::TX::FileFixesSw::m2190 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m2190.dat",
            17921,
            HyoutaUtils::Hash::SHA1FromHexString("d9d215ea34f85fc7af4c3782724bb01eef604968"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[3333333333333333333Q]#4S#800WSo that's...\x01#1000W#5SThat's why I...!"
        // patcher.ReplacePartialCommand(0x1f6d, 0xd7, 0x202f, 0x13, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m2190

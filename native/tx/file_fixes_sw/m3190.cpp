#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m3190[] =
    "Text fixes in Second Spiritron Barrier (boss room).";
}

namespace SenLib::TX::FileFixesSw::m3190 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m3190.dat",
            16073,
            HyoutaUtils::Hash::SHA1FromHexString("829a9692dbf7e5e1ccfa8f75ed9c02250031b0de"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2P#800WHold on just a little longer!#8W\x01#6S#1000WWe're coming to save you!"
        // patcher.ReplacePartialCommand(0x2571, 0x55, 0x25a5, 0x1f, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m3190

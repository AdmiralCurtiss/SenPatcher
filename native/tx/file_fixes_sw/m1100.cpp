#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m1100[] = "Text fixes in Ruins of Abstraction.";
}

namespace SenLib::TX::FileFixesSw::m1100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m1100.dat",
            51745,
            HyoutaUtils::Hash::SHA1FromHexString("a4bd1fb0fba0db6346bb48346c58188d1f5d34a2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1000WI don't have a clue what's happening...#10W\x01#5S#1000WBut stay on your toes!
        // Here they come!"
        // harmless
        // (EV_00_06_01)
        // patcher.ReplacePartialCommand(0x5635, 0x6b, 0x5672, 0x2c, "");

        // "#3C#1C'You can now access the #3CStats#1C and\x01#3CStatus Changes#1C sections under the
        // Help tab."
        // remove the leading "'"
        patcher.RemovePartialCommand(0x9d35, 0x9f, 0x9D3E, 1);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m1100

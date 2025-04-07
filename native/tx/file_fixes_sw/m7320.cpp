#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m7320[] =
    "Text fixes in Fortress of False Gods - First 2.";
}

namespace SenLib::TX::FileFixesSw::m7320 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m7320.dat",
            26945,
            HyoutaUtils::Hash::SHA1FromHexString("4a2f1a63caa717e5af522bfbd7429b7d13aa10e7"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#5K#FSo you were aaffected, too."
        // remove extra 'a'
        // (EV_07_29_00, Asuka must be in party)
        patcher.RemovePartialCommand(0x4d04, 0x2a, 0x4D1E, 1);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m7320

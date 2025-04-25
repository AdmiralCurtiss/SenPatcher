#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3200[] = "Text fixes in Yanagi Sports.";
}

namespace SenLib::TX::FileFixesSw::t3200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3200.dat",
            104225,
            HyoutaUtils::Hash::SHA1FromHexString("4127f845019f3f806e0deaa57c7379ecec101d33"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Kou, Shio, and Sora.."
        // add another period for ellipsis
        // In early Chapter 7, when investigating the 'slashing incident' at Yanagi's (EV_07_03_01).
        // Must have Sora in the party.
        patcher.ExtendPartialCommand(0x7eb5, 0x1a, 0x7ecb, {{0x2e}});

        // the "Obtained Jack-o'-Lantern x2" message is missing a space before the "x2"
        patcher.ExtendPartialCommand(0x15e58, 0x1e, 0x15e6d, {{' '}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3200

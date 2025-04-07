#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m5319[] =
    "Text fixes in Witch's Briar Castle - Midpoint 2.";
}

namespace SenLib::TX::FileFixesSw::m5319 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m5319.dat",
            15673,
            HyoutaUtils::Hash::SHA1FromHexString("cb2ecd6875d7ca3c8c30fc7af5ce6118a63150be"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);


        // The left-in-Japanese "Voice" name tag was fixed in Switch v1.0.1

        // While we're here, improve a linebreak.
        // "#4KUnderstood. You can leave\x01everything to us, then."
        // (EV_04_26_00)
        std::swap(bin[0x3505], bin[0x3510]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m5319

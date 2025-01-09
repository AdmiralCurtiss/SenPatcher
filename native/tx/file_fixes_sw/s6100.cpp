#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s6100[] = "Text fixes in hospital entrance (inside).";
}

namespace SenLib::TX::FileFixesSw::s6100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s6100.dat",
            37713,
            HyoutaUtils::Hash::SHA1FromHexString("40b3ea35854b41540e4efe590b2a7bfc2e629302"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P#1000WTh-This shouldn't be happening!#7W\x01#5S#1000WEverything was going
        // according\x01to plan!"
        // patcher.ReplacePartialCommand(0x3f57, 0x66, 0x3f8e, 0x2d, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s6100

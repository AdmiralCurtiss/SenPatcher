#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s2010[] = "Text fixes in Memorial Park Grove.";
}

namespace SenLib::TX::FileFixesSw::s2010 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s2010.dat",
            64545,
            HyoutaUtils::Hash::SHA1FromHexString("826524586a34aedeb1e4652ddc6cad54a4b35433"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2P(All right... #25W#1000WI've made my choice!)"
        // harmless
        // (EV_16_03_02)
        // patcher.ReplacePartialCommand(0xad0e, 0x3a, 0xad2b, 0x1b, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2010

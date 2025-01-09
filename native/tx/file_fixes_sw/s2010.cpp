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
            64561,
            HyoutaUtils::Hash::SHA1FromHexString("53d974182b18401a6d59b0288ab5479f0f9d1880"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "It's not often someone{n}goes out of their way to call out{n}to me."
        // move linebreak
        std::swap(bin[0xc27a], bin[0xc27e]);

        // "#2P(All right... #25W#1000WI've made my choice!)"
        // patcher.ReplacePartialCommand(0xad30, 0x3a, 0xad4d, 0x1b, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2010

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s7200[] = "Text fixes in hot springs shrine path.";
}

namespace SenLib::TX::FileFixesSw::s7200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s7200.dat",
            44633,
            HyoutaUtils::Hash::SHA1FromHexString("64b63f7a13f6aba6b8134fc6119a66c9011432f8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K#0TLet's see... My family moved there just\nafter I finished elementary school."
        // \n instead of actual newline... curious how this happened
        patcher.ReplacePartialCommand(0x7406, 0xe6, 0x7435, 0x2, {{0x01}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s7200

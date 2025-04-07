#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_nodoka[] =
    "Text fixes in conversations with Nodoka.";
}

namespace SenLib::TX::FileFixesSw::tk_nodoka {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_nodoka.dat",
            15306,
            HyoutaUtils::Hash::SHA1FromHexString("c4c1ed0c779a223ac5af1d3f4056adbeda79fbe3"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "if it's for my dearest Mitsuki,"
        // uncapitalized start of sentence
        // bin[0x8f7] = 0x49;

        // linebreaks
        // std::swap(bin[0x1bd], bin[0x1c3]);
        // std::swap(bin[0x1d9], bin[0x1de]);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_nodoka

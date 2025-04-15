#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t6200[] = "Text fixes in Daikokudou Pawn Shop.";
}

namespace SenLib::TX::FileFixesSw::t6200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6200.dat",
            27121,
            HyoutaUtils::Hash::SHA1FromHexString("0f365634463675a25432580a968cfd4c3df0c9f2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K(That sure is a group… What the\x01hell are they even getting up to?)"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x11cb, 0x4b, 0x11ce, 0x46, "");
        bin[0x11E5] = '.';
        bin[0x11E6] = '.';
        bin[0x11E7] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6200

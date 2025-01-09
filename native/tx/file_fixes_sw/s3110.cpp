#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3110[] = "Text fixes in Acros Theater.";
}

namespace SenLib::TX::FileFixesSw::s3110 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3110.dat",
            160041,
            HyoutaUtils::Hash::SHA1FromHexString("6651fd57eaf2cd85dc688fd1c0cae586f7df1549"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#800WAll right, let's do this...\x01#1000W#5SSPiKA!"
        // patcher.ReplacePartialCommand(0x9e47, 0x3a, 0x9e76, 0x9, "");

        // "#800WIt's called...\x01#1000W"Wish★Wing"!"
        // patcher.ReplacePartialCommand(0x17fbc, 0x37, 0x17fd8, 0x18, "");

        // "#800WIt's called...\x01#1000W"Wish★Wing"!"
        // patcher.ReplacePartialCommand(0x2210a, 0x37, 0x22126, 0x18, "");

        // "We'll leave you guys with a brand\x01new song as our last! We worked\x01hard on it, so I
        // hope you enjoy!"
        // memories menu, mismatches actual game text
        // patcher.ReplacePartialCommand(0x2206d, 0x6d, 0x22070, 0x67, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3110

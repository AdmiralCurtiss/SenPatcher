#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3002[] =
    "Text fixes outside Pandora (after final boss).";
}

namespace SenLib::TX::FileFixesSw::s3002 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3002.dat",
            25673,
            HyoutaUtils::Hash::SHA1FromHexString("da9c84759ab505979f728b7347bc1fe3fad8bd04"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#7K#5SThis joke isn't a funny!"
        // remove 'a'
        // (EV_08_27_04)
        patcher.RemovePartialCommand(0x2eeb, 0x28, 0x2F09, 2);

        // "#2P#800WY-You don't even have to ask!"
        // harmless
        // (EV_08_27_04)
        // patcher.ReplacePartialCommand(0x4806, 0x2f, 0x4811, 0x22, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3002

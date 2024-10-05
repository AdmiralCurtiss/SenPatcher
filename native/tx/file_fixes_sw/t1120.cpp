#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t1120 {
std::string_view GetDescription() {
    return "Text fixes in school 2F.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1120.dat",
            405329,
            HyoutaUtils::Hash::SHA1FromHexString("053c0071dd3283828989191bcea1265bc2bc988c"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#8WWhen's Yukino gonna get in touch with me#1000W...?"
        // remove leftover slow text print command, there's no natural break in the english version
        // of the line to keep it
        patcher.RemovePartialCommand(0x4c838, 0x42, 0x4c843, 0x3);

        // linebreaks
        std::swap(bin[0x4a215], bin[0x4a21c]);
        std::swap(bin[0x4c12c], bin[0x4c129]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1120

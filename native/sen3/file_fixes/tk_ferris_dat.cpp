#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_ferris[] =
    "Fix sentence in conversation with Ferris.";
}

namespace SenLib::Sen3::FileFixes::tk_ferris_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_ferris.dat",
            6384,
            HyoutaUtils::Hash::SHA1FromHexString("2e126f3f3b84587d13d1c22dec296ce24b39fb97"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kHow are the those two being treated?"
        // the those two -> those two
        // (chapter 4, 7/15 afternoon, in Hotel Valar)
        patcher.RemovePartialCommand(0x673, 0x9e, 0x684, 4);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_ferris_dat

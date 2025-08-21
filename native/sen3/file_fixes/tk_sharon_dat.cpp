#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_sharon[] =
    "Fix typography error in conversation with Sharon.";
}

namespace SenLib::Sen3::FileFixes::tk_sharon_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_sharon.dat",
            2449,
            HyoutaUtils::Hash::SHA1FromHexString("a8cd57ea0f5a3d2865adc2b3b98b1fb4d1d67017"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kOh, well..thank you so much."
        // missing dot for ellipsis
        patcher.ExtendPartialCommand(0x67d, 0x4b, 0x690, {{'.'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_sharon_dat

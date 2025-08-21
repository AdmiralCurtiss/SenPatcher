#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_tatiana[] =
    "Fix typography error in conversation with Tatiana.";
}

namespace SenLib::Sen3::FileFixes::tk_tatiana_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_tatiana.dat",
            24553,
            HyoutaUtils::Hash::SHA1FromHexString("a9c735f042b503a67a19f9e7671b2cdf4318964f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_4#M_A#B_0P-Please accept this as a token of\x01my thanks.."
        // extra dot
        // (two instances)
        patcher.RemovePartialCommand(0x5498, 0xcb, 0x5560, 1);
        patcher.RemovePartialCommand(0x557a, 0xcf, 0x5646, 1);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_tatiana_dat

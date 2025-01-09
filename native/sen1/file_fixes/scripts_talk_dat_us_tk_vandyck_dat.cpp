#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_vandyck[] =
    "Fix formatting issues in conversation with Vandyck.";
}

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_vandyck_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_vandyck.dat",
            17811,
            HyoutaUtils::Hash::SHA1FromHexString("b838141d25f707a7c95191db2f8c324a3e0a34c0"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x1312, 0x183, 0x1420, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_vandyck_dat

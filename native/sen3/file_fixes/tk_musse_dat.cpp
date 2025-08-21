#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_musse[] = "Fix typo in conversation with Musse.";
}

namespace SenLib::Sen3::FileFixes::tk_musse_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_musse.dat",
            19617,
            HyoutaUtils::Hash::SHA1FromHexString("ec12b1b1eb405eb2df1b68a659de20fe08e15b62"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#k#1U(Hmm... I get they feeling they're\x01hiding something, though...)"
        // they feeling -> the feeling
        // (chapter 4, 7/15 train during the trip to Heimdallr)
        patcher.RemovePartialCommand(0x977, 0x4d, 0x994, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_musse_dat

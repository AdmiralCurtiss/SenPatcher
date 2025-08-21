#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_stark[] =
    "Fix terminology consistency issues in conversations with Stark.";
}

namespace SenLib::Sen3::FileFixes::tk_stark_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_stark.dat",
            29257,
            HyoutaUtils::Hash::SHA1FromHexString("095615a115f3653b2ecb59a16a756f8c3f12b68d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress (chapter 3 field exercises day 1, camp)
        patcher.RemovePartialCommand(0x5eb3, 0x152, 0x5f31, 0x4);

        // "This is a lot faster than usual trip.\x01We'll be there in no time."
        // than usual -> than the usual
        // (chapter 4, 7/15 train during the trip to Heimdallr)
        patcher.ExtendPartialCommand(0x990, 0x89, 0x9f1, STR_SPAN("the "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_stark_dat

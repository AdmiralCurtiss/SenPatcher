#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_shanshan[] =
    "Fix typo in conversation with Shanshan.";
}

namespace SenLib::Sen3::FileFixes::tk_sansan_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_sansan.dat",
            3009,
            HyoutaUtils::Hash::SHA1FromHexString("d4e69d9cbf9f7a6b3dfaad197ea8b386ba00debc"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kCome to think of it, I haven't seen\x01Rixia in awhile."
        // awhile -> a while
        // (chapter 2, 5/20 afternoon 2nd conversation, in her restaurant)
        patcher.ExtendPartialCommand(0x900, 0x6c, 0x937, {{' '}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_sansan_dat

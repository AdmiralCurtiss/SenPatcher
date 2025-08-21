#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_lughman[] =
    "Fix typography errors in conversations with Lughman.";
}

namespace SenLib::Sen3::FileFixes::tk_alberich_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_alberich.dat",
            2681,
            HyoutaUtils::Hash::SHA1FromHexString("0588cbb03748da2b92d721c9b310110cd2278a92"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kI’m sorry to ask this of you, but\x01we've no time to waste."
        // wrong apostrophe
        // (chapter 4, right before the dark dragon's nest, enter the underground then go back out
        // into the museum and talk to alberich twice)
        patcher.ReplacePartialCommand(0xd7, 0xab, 0xe1, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_alberich_dat

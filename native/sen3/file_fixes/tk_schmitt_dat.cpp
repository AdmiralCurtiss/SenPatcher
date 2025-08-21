#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_schmidt[] = "Fix typo in conversation with Schmidt.";
}

namespace SenLib::Sen3::FileFixes::tk_schmitt_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_schmitt.dat",
            25929,
            HyoutaUtils::Hash::SHA1FromHexString("059b7da5d5ddac7a7a9ef56efdb74726a295489f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kAh... Now that's good cup of coffee."
        // good cup -> a good cup
        // (chapter 4, 7/8 night, Recette bakery)
        patcher.ExtendPartialCommand(0x22ed, 0x6e, 0x2307, STR_SPAN("a "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_schmitt_dat

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3230[] =
    "Fix missing word in Ordis (Count Egret's Residence)";
}

namespace SenLib::Sen3::FileFixes::t3230_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3230.dat",
            18276,
            HyoutaUtils::Hash::SHA1FromHexString("a596971f1843163939996fc6df7a92fae5786ad4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E[1]#M_4#B_0I've known her since she was child, due to being\x01acquainted with the
        // previous count. I never\x01thought she would gain such a name for herself!"
        // she was child -> she was a child
        // (chapter 3, 6/17 morning, cutscene when first meeting Count Egret)
        patcher.ExtendPartialCommand(0x38aa, 0xc9, 0x3902, STR_SPAN("a "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3230_dat

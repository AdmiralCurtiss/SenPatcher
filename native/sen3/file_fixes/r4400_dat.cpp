#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r4400[] = "Fix missing word in West Ostia Highway";
}

namespace SenLib::Sen3::FileFixes::r4400_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r4400.dat",
            136369,
            HyoutaUtils::Hash::SHA1FromHexString("a9a7ea6d43b59c29bd672c44e5e4c03d9d988b57"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "W-Well, I was thinking that maybe the\x014th Armored Division could take care it..."
        // take care it -> take care of it
        // (chapter 4, 7/17 after the Lechter quest, when starting the monster quest here)
        patcher.ExtendPartialCommand(0xfbf8, 0x8d, 0xfc4a, STR_SPAN("of "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r4400_dat

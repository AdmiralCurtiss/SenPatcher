#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c3000[] =
    "Fix name consistency issues in Heimdallr (Sankt District).";
}

namespace SenLib::Sen3::FileFixes::c3000_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c3000.dat",
            332825,
            HyoutaUtils::Hash::SHA1FromHexString("6c8a21013585ce0f1ddf378cf8963aec5f283a75"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Miss Clotilde -> Ms. Clotilde
        patcher.ReplacePartialCommand(0x1c3f6, 0x4c, 0x1c433, 0x3, {{0x73, 0x2e}});

        // Sir Hemisphere -> Father Wazy
        patcher.ReplacePartialCommand(
            0x25615,
            0x89,
            0x25648,
            0xe,
            {{0x46, 0x61, 0x74, 0x68, 0x65, 0x72, 0x20, 0x57, 0x61, 0x7a, 0x79}});

        // "#K#0TGovernor......"
        // way too many dots
        // (chapter 4, 7/16 morning, during the St. Astraia's Distress sidequest)
        patcher.RemovePartialCommand(0x30f98, 0x1c, 0x30faf, 3);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c3000_dat

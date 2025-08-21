#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_maya[] = "Fix typo in conversation with Maya.";
}

namespace SenLib::Sen3::FileFixes::tk_maya_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_maya.dat",
            14049,
            HyoutaUtils::Hash::SHA1FromHexString("097807720abf5c7da0898a42c41b43e275f48bb5"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kA free day and club activites...\x01Tomorrow is going to be a good day."
        // activites -> activities
        // (chapter 1, 4/15 evening, dorm 1F)
        patcher.ExtendPartialCommand(0x3498, 0x4f, 0x34bc, {{'i'}});

        // "#kAhh.. The pancakes at\x01Recette are really good."
        // missing dot
        // (chapter 3, 6/10 evening, Recette bakery)
        patcher.ExtendPartialCommand(0x19e6, 0x62, 0x19f4, {{'.'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_maya_dat

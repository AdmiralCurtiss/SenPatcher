#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_rosine[] =
    "Fix typo and factual error in conversations with Rosine.";
}

namespace SenLib::Sen3::FileFixes::tk_rosine_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_rosine.dat",
            16945,
            HyoutaUtils::Hash::SHA1FromHexString("113e545124a0990343e451554dd94ecbf410bffd"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#k#1U(For someone so smart, she can be a little\x01oblivous...)"
        // oblivous -> oblivious
        // (chapter 2, 5/14 morning, Leeves church)
        patcher.ExtendPartialCommand(0x24f2, 0x9c, 0x252e, {{'i'}});

        // "#E_8All the more worrisome when last month and the\x01month before, similar phenomena
        // occurred in\x01Sutherland and Crossbell respectively."
        // this is the wrong way around. last month was Crossbell and the month before was
        // Sutherland
        // (chapter 3, 6/11 evening, Leeves church)
        // patcher.ReplacePartialCommand(0x12ee, 0x111, 0x1378, 0x85, "");
        HyoutaUtils::Vector::ShiftData(bin, 0x13e2, 0x13f0, 4);
        HyoutaUtils::Vector::ShiftData(bin, 0x13d7, 0x13f0, 11);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_rosine_dat

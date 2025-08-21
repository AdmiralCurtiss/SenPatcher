#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_v0040[] = "Fix typography in Derfflinger Car 5.";
}

namespace SenLib::Sen3::FileFixes::v0040_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0040.dat",
            28965,
            HyoutaUtils::Hash::SHA1FromHexString("7f95a95868434420635373d78515903ea8597f59"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Y-Yeah... Still, that’s no testament\x01against his mechanical genius!"
        // wrong apostrophe
        // (chapter 2, 5/20, after geofront, talk to alisa/tita)
        patcher.ReplacePartialCommand(0x4f55, 0xd1, 0x4f71, 3, {{'\''}});

        // "#7KCan’t wait to see what\x01you all think of it."
        // wrong apostrophe
        patcher.ReplacePartialCommand(0x5bf4, 0x39, 0x5c01, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0040_dat

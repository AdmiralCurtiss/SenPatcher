#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2830[] = "Text fixes in Heimdallr (Rieveldt Company)";
}

namespace SenLib::Sen3::FileFixes::c2830_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2830.dat",
            34105,
            HyoutaUtils::Hash::SHA1FromHexString("dccfb2311bfa16903b1b12b446deb2eda97dea91"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_2#M_A#B_0I have to set a good example for the\x01my underclassmen at the academy."
        // the my -> the
        // I'm not entirely sure if 'underclassmen' is the right term to use but whatever...
        // (chapter 4, 7/16 morning, NPC dialogue with Kalinka)
        patcher.RemovePartialCommand(0x1437, 0xff, 0x1514, 3);

        // "#E[1]#M_A#B_0All you have to do is breath in and out.\x01It's an easy instrument to pick
        // up."
        // breath -> breathe
        // (chapter 4, 7/17 free time, NPC dialogue with Adams)
        patcher.ExtendPartialCommand(0x2684, 0x128, 0x26ef, {{'e'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2830_dat

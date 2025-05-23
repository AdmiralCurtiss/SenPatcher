#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_e7101[] =
    "Fix incorrect voice clip being used near end of Intermission.";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7101_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/e7101.dat",
            67825,
            HyoutaUtils::Hash::SHA1FromHexString("cc2ff5a5eadebb18a6db19106284dd35ac201243"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // remove stray space at start of textbox
        patcher.RemovePartialCommand(0x107b, 0x18, 0x1083, 1);

        // remove incorrect 'Your Lordship!' clip from Rean
        // this texbox has two voice clips assigned to, the remaining one is the correct one
        patcher.RemovePartialCommand(0x64e9, 0x22, 0x64f4, 5);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7101_dat

#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t5501_dat {
std::string_view GetDescription() {
    return "Add voice clips in scene at Garrelia Fortress.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t5501.dat",
            111881,
            SenPatcher::SHA1FromHexString("df089163a734c202ebb8e05491ff6537fb1dbff7"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // add voice clips
        // Both of these are in the scenes at the end of Act 1 Part 1, with the Xeno/Leo fight.
        patcher.ExtendPartialCommand(0x7941, 0x10, 0x794c, {{0x11, 0x2d, (char)0xff, 0x00, 0x00}});
        patcher.ExtendPartialCommand(0xa8a5, 0x1a, 0xa8b1, {{0x11, 0x5d, (char)0xff, 0x00, 0x00}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t5501_dat

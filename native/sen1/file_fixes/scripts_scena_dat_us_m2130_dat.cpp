#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m2130_dat {
std::string_view GetDescription() {
    return "Text/Voice match in Heimdallr catacombs.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/m2130.dat",
            60605,
            SenPatcher::SHA1FromHexString("a2fcccff0c7877e4b565ed931b62f6a03191c289"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // remove stutter from text that isn't in voice
        patcher.RemovePartialCommand(0x4865, 0x38, 0x486f, 0x3);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m2130_dat

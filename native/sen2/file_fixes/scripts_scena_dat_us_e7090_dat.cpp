#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7090_dat {
std::string_view GetDescription() {
    return "Add unused grunt to Jusis' line in scene on Courageous after Celdic.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/e7090.dat",
            132521,
            SenPatcher::SHA1FromHexString("34005eaf8e8f7823f5079700cf130b2b3b73d047"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // add Jusis grunt voice clip
        patcher.ExtendPartialCommand(0x161bb, 0xd, 0x161c3, {{0x11, 0x50, (char)0xff, 0x00, 0x00}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7090_dat

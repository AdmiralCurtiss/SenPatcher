#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t0001_dat {
std::string_view GetDescription() {
    return "Text/voice match in epilogue NG+ only scene.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0001.dat",
            123881,
            SenPatcher::SHA1FromHexString("9ba616b8ea524fff05a5ac7e709f4c5fa4561a4b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // What -> Wh-What
        // Actually on further inspection the stutter isn't really audible in game, so let's not.
        // patcher.ExtendPartialCommand(0x18ada, 0x6f, 0x18ae9, {{ 0x2d, 0x57, 0x68 }});

        // Remove clip on Rosine's last '...', PS4 does this and it *is* kinda out of place and also
        // duplicated from earlier in the scene.
        patcher.RemovePartialCommand(0x1b2e3, 0x12, 0x1b2e6, 5);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t0001_dat

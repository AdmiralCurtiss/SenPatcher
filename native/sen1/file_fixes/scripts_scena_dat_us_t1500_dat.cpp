#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1500_dat {
std::string_view GetDescription() {
    return "Fix formatting issues and text/voice mismatch in Celdic.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1500.dat",
            203318,
            SenPatcher::SHA1FromHexString("4f2a0d4b4c1602bbd9aec8dc0785f334c6c285f9"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.ReplacePartialCommand(0x11129, 0x34, 0x11136, 3, {{0x27}});
        patcher.ReplacePartialCommand(
            0x2fc1d, 0xf, 0x2fc25, 2, {{0x52, 0x2d, 0x52, 0x69, 0x67, 0x68, 0x74}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1500_dat

#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m0040_dat {
std::string_view GetDescription() {
    return "Fix text/voice mismatch in Lunaria Nature Park.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/m0040.dat",
            35926,
            SenPatcher::SHA1FromHexString("c377fcdc74a195f79dba543e80a70d2eb7fdf12a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.ReplacePartialCommand(0x67a4, 0x16, 0x67ad, 7, {{0x41, 0x68}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m0040_dat

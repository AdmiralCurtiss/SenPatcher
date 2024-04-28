#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_c0110_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Le Sage (Heimdallr).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/c0110.dat",
            34872,
            SenPatcher::SHA1FromHexString("a740904f6ad37411825565981daa5f5915b55b69"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.ReplacePartialCommand(0xb2f, 0x61, 0xb34, 3, {{0x27}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_c0110_dat

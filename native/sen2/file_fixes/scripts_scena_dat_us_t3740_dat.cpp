#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3740_dat {
std::string_view GetDescription() {
    return "Typo fixes in Roer IoT.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t3740.dat",
            33201,
            SenPatcher::SHA1FromHexString("985274ffb868643445b91d5f76be284ac2c79a1c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // All of these fix Gregor's name from 'Grego' to 'Gregor'
        patcher.ExtendPartialCommand(0x36bb, 0x141, 0x36ce, {{0x72}});
        patcher.ExtendPartialCommand(0x3815, 0x73, 0x382f, {{0x72}});
        patcher.ExtendPartialCommand(0x38cd, 0xbe, 0x3916, {{0x72}});
        patcher.ExtendPartialCommand(0x39a4, 0xba, 0x39b7, {{0x72}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3740_dat

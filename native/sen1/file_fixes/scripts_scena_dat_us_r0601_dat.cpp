#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0601_dat {
std::string_view GetDescription() {
    return "Fix text issues in 1st scene of bike events.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r0601.dat",
            37372,
            SenPatcher::SHA1FromHexString("531ae02b784b6530f4dc08676a793c89f9ebae68"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // missing period
        patcher.ExtendPartialCommand(0x1fdb, 0x5b, 0x2034, {{0x2e}});

        // missing word
        patcher.ExtendPartialCommand(0x3962, 0x42, 0x3995, {{0x62, 0x65, 0x20}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0601_dat

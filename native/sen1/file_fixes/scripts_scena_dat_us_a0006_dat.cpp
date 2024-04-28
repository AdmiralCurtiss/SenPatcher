#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_a0006_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in debug map.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/a0006.dat",
            28887,
            SenPatcher::SHA1FromHexString("73fd289006017eab8816636d998d21aa8fb38d68"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0x3a67, 0x6f, 0x3ac4, 1);
        patcher.RemovePartialCommand(0x3e5f, 0x5f, 0x3e8c, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_a0006_dat

#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0800_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in southern Nord plains.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r0800.dat",
            113613,
            SenPatcher::SHA1FromHexString("48d8e5ebfacb29fed7c5c4e75c84e108277cb5fe"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // double space
        SenScriptPatcher patcher(bin);
        patcher.RemovePartialCommand(0xf877, 0x46, 0xf89c, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0800_dat

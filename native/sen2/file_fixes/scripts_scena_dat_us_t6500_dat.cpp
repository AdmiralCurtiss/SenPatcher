#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t6500_dat {
std::string_view GetDescription() {
    return "Missing voice clip at Schwarz Drache Barrier.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t6500.dat",
            81833,
            SenPatcher::SHA1FromHexString("a7bff27d9558648cb36dde4ab7cf4e9b1ca120c2"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // "Your Lordship!" line doesn't have its voice clip, probably because it was accidentally
        // assigned to Rean in a different scene
        patcher.ExtendPartialCommand(0x5f8f, 0x1e, 0x5f92, {{0x11, 0x19, (char)0xff, 0x00, 0x00}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t6500_dat

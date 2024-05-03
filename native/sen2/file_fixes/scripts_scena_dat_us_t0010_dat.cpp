#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t0010_dat {
std::string_view GetDescription() {
    return "Fix extra word in Thors (bottom floor).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0010.dat",
            31169,
            HyoutaUtils::Hash::SHA1FromHexString("7f1a94208801acaf1a3ba2fd7cba496444904a1b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // during the our days in the army -> during our days in the army
        // This is during the very first chance you get to walk around in the Finale, in Vandyck's
        // office.
        patcher.RemovePartialCommand(0x4201, 0x11b, 0x42dc, 0x4);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t0010_dat

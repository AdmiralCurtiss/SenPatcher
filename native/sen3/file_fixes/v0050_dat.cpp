#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_v0050[] =
    "Fix terminology consistency issues in Derfflinger Car 6.";
}

namespace SenLib::Sen3::FileFixes::v0050_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0050.dat",
            53281,
            HyoutaUtils::Hash::SHA1FromHexString("b1f43611bbf1130ff552405bc50d2f75637e49e1"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // synch -> sync
        patcher.RemovePartialCommand(0xb533, 0x185, 0xb62c, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0050_dat

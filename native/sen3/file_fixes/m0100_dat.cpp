#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m0100[] =
    "Fix terminology consistency issues in Einhel Lv1.";
}

namespace SenLib::Sen3::FileFixes::m0100_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m0100.dat",
            41553,
            HyoutaUtils::Hash::SHA1FromHexString("b0f3a274fd1e54528118b4a68706e7c330285fef"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // synch -> sync
        patcher.RemovePartialCommand(0x4a10, 0xc8, 0x4ac6, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m0100_dat

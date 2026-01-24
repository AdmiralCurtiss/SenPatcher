#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2030[] =
    "Fix text issues in Parm (Dwight Arms & Orbal Factory)";
}

namespace SenLib::Sen3::FileFixes::t2030_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t2030.dat",
            3297,
            HyoutaUtils::Hash::SHA1FromHexString("943fca97576e2bd70cc841e866030d32c254f68f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "They used to say bad spirits come\x01out at night,  so we never went\x01out after
        // sunset."
        // double space
        // Chapter 1 field trip first day, before returning to camp
        patcher.RemovePartialCommand(0x8f0, 0x97, 0x926, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t2030_dat

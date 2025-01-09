#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0010[] = "Terminology fixes in Branch Campus Dorm.";
}

namespace SenLib::Sen3::FileFixes::t0010_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0010.dat",
            367201,
            HyoutaUtils::Hash::SHA1FromHexString("53812a2a94c94c7bc9068c4e43d8973eb32d5034"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // Master Yun -> Master Ka-fai
        patcher.ReplacePartialCommand(
            0x1be3a, 0x50, 0x1be84, 0x3, {{0x4b, 0x61, 0x2d, 0x66, 0x61, 0x69}});

        // Hah. Guess so... -> Hmph. True. (to match existing voice line)
        patcher.ReplacePartialCommand(
            0x39b4e, 0x1e, 0x39b5b, 0xe, {{0x6d, 0x70, 0x68, 0x2e, 0x20, 0x54, 0x72, 0x75, 0x65}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0010_dat

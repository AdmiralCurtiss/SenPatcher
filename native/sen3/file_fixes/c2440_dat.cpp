#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2440[] =
    "Fix terminology consistency issues in Valflame Palace (back area).";
}

namespace SenLib::Sen3::FileFixes::c2440_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2440.dat",
            87569,
            HyoutaUtils::Hash::SHA1FromHexString("37161993d10c65821c11026fedb1af5290ceff4d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Highness -> Majesty
        // rean to eugent
        patcher.ReplacePartialCommand(
            0x30a9, 0x47, 0x30e2, 0x8, {{0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}});
        // rean about eugent
        patcher.ReplacePartialCommand(
            0x52f6, 0xd4, 0x5393, 0x8, {{0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}});

        // royal family -> Imperial family
        patcher.ReplacePartialCommand(
            0x45d8, 0x1c2, 0x46ce, 0x3, {{0x49, 0x6d, 0x70, 0x65, 0x72, 0x69}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2440_dat

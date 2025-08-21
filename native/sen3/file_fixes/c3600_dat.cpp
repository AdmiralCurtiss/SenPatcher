#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c3600[] = "Fix typo in Karel Imperial Villa (outside)";
}

namespace SenLib::Sen3::FileFixes::c3600_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c3600.dat",
            48641,
            HyoutaUtils::Hash::SHA1FromHexString("544d40798902e950b6aa300d3f074be34e766615"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#K#0TWoah..."
        // should be 'Whoa' for consistency
        // (chapter 4, 7/16 morning, during the scene meeting with Gaius and Elliot)
        // patcher.ReplacePartialCommand(0x81f9, 0x15, 0x8202, 0xa, "");
        HyoutaUtils::Vector::ShiftData(bin, 0x8208, 0x8206, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c3600_dat

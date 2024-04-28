#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::r2290_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Crossbell (Ex. Camp).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r2290.dat",
            138561,
            SenPatcher::SHA1FromHexString("0c5fbbb90cb3459e0d2f5b9769eb5f7d9d87992c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // royal family -> Imperial family
        patcher.ReplacePartialCommand(
            0x188ae, 0x53, 0x188d2, 0x3, {{0x49, 0x6d, 0x70, 0x65, 0x72, 0x69}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r2290_dat

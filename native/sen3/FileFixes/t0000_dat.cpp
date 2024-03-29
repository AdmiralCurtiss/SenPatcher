#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::t0000_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Leeves.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0000.dat",
            336305,
            SenPatcher::SHA1FromHexString("895e048dd4b006b1bda8d2434de9edfb20142ef9"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Sir Thomas -> Father Thomas
        patcher.ReplacePartialCommand(
            0x2dbe1, 0x81, 0x2dbfc, 0x2, {{0x46, 0x61, 0x74, 0x68, 0x65}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0000_dat

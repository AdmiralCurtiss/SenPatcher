#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::m0000_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Einhel Lv0.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m0000.dat",
            80905,
            SenPatcher::SHA1FromHexString("289204f27043cf2fdc6272bffd1ec0857f3230d3"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // synch -> sync
        patcher.RemovePartialCommand(0x9001, 0x4a, 0x9032, 0x1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m0000_dat

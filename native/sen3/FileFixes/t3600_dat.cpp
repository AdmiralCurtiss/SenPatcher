#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sen/sen_script_patcher.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::t3600_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Ordis (Harbor District).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3600.dat",
            148257,
            SenPatcher::SHA1FromHexString("f20f8c36a44c88fecc44155250b42fa0259cd699"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0xb336, 0x17e, 0xb3b6, 0x4);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3600_dat

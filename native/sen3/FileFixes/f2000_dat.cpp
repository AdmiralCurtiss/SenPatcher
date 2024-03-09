#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sen/sen_script_patcher.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::f2000_dat {
std::string_view GetDescription() {
    return "Fix Hamilton gender (scene 2, text only).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/f2000.dat",
            88801,
            SenPatcher::SHA1FromHexString("bb3220cb9f85e554fd1196688bcc1ea578a3d234"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // switch gender in line from Angelica
        patcher.ExtendPartialCommand(0x46cf, 0x1b0, 0x482b, {{0x73}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::f2000_dat

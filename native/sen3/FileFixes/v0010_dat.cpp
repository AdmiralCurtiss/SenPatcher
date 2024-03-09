#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sen/sen_script_patcher.h"
#include "../../sha1.h"

namespace SenLib::Sen3::FileFixes::v0010_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Derfflinger Car 1.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0010.dat",
            293281,
            SenPatcher::SHA1FromHexString("4c2781204c47f2457e831b6a50413068b498c5b3"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // the Juno Naval Fortress -> Juno Naval Fortress
        patcher.RemovePartialCommand(0x26097, 0xe7, 0x26147, 0x4);
        patcher.RemovePartialCommand(0x262c6, 0x25, 0x262d0, 0x4);

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0010_dat

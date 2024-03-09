#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"
#include "../../util/vector.h"

namespace SenLib::Sen3::FileFixes::m3000_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Juno Naval Fortress (Sub 1).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m3000.dat",
            54289,
            SenPatcher::SHA1FromHexString("03bec649d56e048772de4fd1ce013005b651d784"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x6847, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m3000_dat

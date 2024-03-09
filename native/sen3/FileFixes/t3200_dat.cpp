#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"
#include "../../util/vector.h"

namespace SenLib::Sen3::FileFixes::t3200_dat {
std::string_view GetDescription() {
    return "Aurelia/Bardias name fixes in Ordis (North Street).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3200.dat",
            50273,
            SenPatcher::SHA1FromHexString("11f838467868bad61623e965fbc3c0607d2a3356"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Brigadier General Wallace -> Brigadier General Bardias
        WriteAt(bin, 0x6057, {{0x42, 0x61, 0x72, 0x64, 0x69, 0x61, 0x73}});

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x60a9, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x9e1b, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3200_dat

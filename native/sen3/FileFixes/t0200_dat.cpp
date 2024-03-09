#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"
#include "../../util/vector.h"

namespace SenLib::Sen3::FileFixes::t0200_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Branch Campus grounds.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0200.dat",
            541089,
            SenPatcher::SHA1FromHexString("77098ac08dc593d1b4eb1f2be2b5fe5665de36e4"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Your Majesty's -> Your Highness'
        WriteAt(bin, 0x2506e, {{0x48, 0x69, 0x67, 0x68, 0x6e, 0x65, 0x73, 0x73, 0x27}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0200_dat

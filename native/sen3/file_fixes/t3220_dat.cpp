#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::t3220_dat {
std::string_view GetDescription() {
    return "Aurelia name fixes in Ordis (Orbal Factory).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3220.dat",
            18977,
            SenPatcher::SHA1FromHexString("4cdac7c910724aa27bfe5ada8fc25f9b9118833b"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x141e, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});
        WriteAt(bin, 0x1e43, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3220_dat

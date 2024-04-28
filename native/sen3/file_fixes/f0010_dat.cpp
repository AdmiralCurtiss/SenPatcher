#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::Sen3::FileFixes::f0010_dat {
std::string_view GetDescription() {
    return "Aurelia name fix in Dreknor Fortress.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/f0010.dat",
            18905,
            SenPatcher::SHA1FromHexString("1c5d1ffde19b3fe0b14ce4b4d8c29ebae850b8cb"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // General Aurelia -> General Le Guin
        WriteAt(bin, 0x2330, {{0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::f0010_dat

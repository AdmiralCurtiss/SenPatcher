#include <filesystem>
#include <string_view>
#include <vector>

#include "../file_fixes.h"

#include "../../p3a/pack.h"
#include "../../p3a/structs.h"
#include "../../sha1.h"
#include "../../util/vector.h"

namespace SenLib::Sen3::FileFixes::c0420_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Crossbell (Fisherman's Guild).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0420.dat",
            8242,
            SenPatcher::SHA1FromHexString("4d4183d8d45f846c7659a7db1d8c39c758ffb50a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // expert angler -> Master Fisher
        SenLib::WriteAt(
            bin, 0x1837, {{0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x46, 0x69, 0x73, 0x68}});

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0420_dat

#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::c0430_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Crossbell (Business Owners' Association).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c0430.dat",
            18617,
            SenPatcher::SHA1FromHexString("2a1b99c4395efd230bc10942ae1c06f436f4e504"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Business Owner's Association -> Business Owners' Association
        std::swap(bin[0xd40], bin[0xd41]);
        std::swap(bin[0x1493], bin[0x1494]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c0430_dat

#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen3::FileFixes::t3510_dat {
std::string_view GetDescription() {
    return "Terminology fixes in Ordis (Duke's Castle Entrance).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t3510.dat",
            64009,
            SenPatcher::SHA1FromHexString("7d6eddb306de371fd7fcfd55b599301683e18853"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // the port city -> the Port City
        bin[0x4401] = 0x50;
        bin[0x4406] = 0x43;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t3510_dat

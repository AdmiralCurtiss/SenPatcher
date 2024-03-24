#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0610_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in eastern Trista highway.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r0610.dat",
            100105,
            SenPatcher::SHA1FromHexString("b96565a04c292ef7de28bbf071c5eae22dddfffe"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // formatting issues in Sara Chapter 4 Day bonding event
        std::swap(bin[0x18245], bin[0x1824d]);
        std::swap(bin[0x182ac], bin[0x182b0]);
        std::swap(bin[0x184d6], bin[0x184db]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_r0610_dat

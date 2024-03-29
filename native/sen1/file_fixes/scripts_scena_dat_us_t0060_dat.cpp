#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0060_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Library.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0060.dat",
            94450,
            SenPatcher::SHA1FromHexString("2c67ece34e81dfc667fd57699b818fcfd4e6b06e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // formatting issues in Jusis Chapter 2 Evening bonding event
        std::swap(bin[0x10d09], bin[0x10d0c]);
        std::swap(bin[0x10e1a], bin[0x10e1f]);

        // formatting issues in Towa Chapter 5 Evening bonding event
        WriteUInt8(&bin[0x129ff], 0x01);
        std::swap(bin[0x12f39], bin[0x12f3e]);
        std::swap(bin[0x130c3], bin[0x130c6]);
        std::swap(bin[0x1343f], bin[0x1344b]);
        std::swap(bin[0x13465], bin[0x13475]);
        std::swap(bin[0x134a7], bin[0x134b1]);
        std::swap(bin[0x13861], bin[0x13866]);
        std::swap(bin[0x13aac], bin[0x13ab1]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0060_dat

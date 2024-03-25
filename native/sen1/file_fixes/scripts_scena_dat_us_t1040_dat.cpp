#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1040_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Le Sage (Trista).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1040.dat",
            27140,
            SenPatcher::SHA1FromHexString("0c8591be2e25657dfecdb3ead94e046c0bf08069"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // formatting issues in Alisa Chapter 4 Day bonding event
        std::swap(bin[0x4b0e], bin[0x4b14]);
        std::swap(bin[0x4ca7], bin[0x4cab]);
        std::swap(bin[0x4cf4], bin[0x4cf9]);
        std::swap(bin[0x50c8], bin[0x50cd]);

        // formatting issues in Millium Chapter 5 Day bonding event
        std::swap(bin[0x5ab0], bin[0x5ab4]);
        std::swap(bin[0x5ad6], bin[0x5ada]);
        std::swap(bin[0x5b1f], bin[0x5b26]);
        WriteUInt8(&bin[0x5b5d], 0x01);
        std::swap(bin[0x5bde], bin[0x5be6]);
        std::swap(bin[0x5d8c], bin[0x5d91]);
        std::swap(bin[0x5dd3], bin[0x5dd8]);
        std::swap(bin[0x5e3d], bin[0x5e42]);
        std::swap(bin[0x67dd], bin[0x67e3]);
        std::swap(bin[0x68b5], bin[0x68ba]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1040_dat

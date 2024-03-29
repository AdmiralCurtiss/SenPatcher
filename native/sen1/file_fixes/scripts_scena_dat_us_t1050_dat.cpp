#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1050_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Kirsche's.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1050.dat",
            82800,
            SenPatcher::SHA1FromHexString("60ff5f6535daa1debc7ecab2d514a387eeecb587"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // formatting issues in Crow Chapter 2 Day bonding event
        std::swap(bin[0x12665], bin[0x12669]);
        WriteUInt8(&bin[0x1268e], 0x01);
        std::swap(bin[0x128c6], bin[0x128cd]);
        WriteUInt8(&bin[0x128e8], 0x01);
        WriteUInt8(&bin[0x12a3e], 0x01);

        // formatting issues in Machias Chapter 4 Day bonding event
        std::swap(bin[0xf9f8], bin[0xf9fa]);
        std::swap(bin[0xfa9f], bin[0xfaa8]);
        std::swap(bin[0xfac4], bin[0xfad2]);
        std::swap(bin[0xfb8d], bin[0xfb93]);
        std::swap(bin[0xfe48], bin[0xfe4d]);
        std::swap(bin[0xfe98], bin[0xfe9e]);
        std::swap(bin[0x100a9], bin[0x100b0]);
        std::swap(bin[0x100cf], bin[0x100d8]);

        // formatting issues in Gaius Chapter 5 Day bonding event
        std::swap(bin[0x109d0], bin[0x109d3]);
        WriteUInt8(&bin[0x10c48], 0x01);
        std::swap(bin[0x10d5b], bin[0x10d60]);
        std::swap(bin[0x117bf], bin[0x117c3]);

        // formatting issues in Sara Chapter 6 Day bonding event
        WriteUInt8(&bin[0x13713], 0x01);
        WriteUInt8(&bin[0x137d7], 0x01);
        std::swap(bin[0x138e7], bin[0x138ec]);
        WriteUInt8(&bin[0x139a5], 0x01);
        WriteUInt8(&bin[0x13b38], 0x01);
        std::swap(bin[0x13bce], bin[0x13bd2]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1050_dat

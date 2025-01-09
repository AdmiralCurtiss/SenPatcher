#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1030[] = "Fix formatting issues in Keynes'.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1030_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1030.dat",
            29584,
            HyoutaUtils::Hash::SHA1FromHexString("954a31dd3eaf244f159fbe83607ae870f3bdc89f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // formatting issues in Emma Chapter 2 Day bonding event
        std::swap(bin[0x4db7], bin[0x4dbc]);
        std::swap(bin[0x4de2], bin[0x4de6]);
        WriteUInt8(&bin[0x5046], 0x01);
        std::swap(bin[0x507b], bin[0x5080]);

        // formatting issues in Machais Chapter 6 Day bonding event
        std::swap(bin[0x3f0d], bin[0x3f10]);
        std::swap(bin[0x3f51], bin[0x3f59]);
        std::swap(bin[0x40ba], bin[0x40c1]);
        std::swap(bin[0x40e1], bin[0x40ec]);
        std::swap(bin[0x42ba], bin[0x42be]);
        std::swap(bin[0x42df], bin[0x42e6]);
        std::swap(bin[0x4491], bin[0x4496]);
        std::swap(bin[0x454e], bin[0x4553]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1030_dat

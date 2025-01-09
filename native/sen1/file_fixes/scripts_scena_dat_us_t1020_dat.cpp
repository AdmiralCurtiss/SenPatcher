#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1020[] = "Fix formatting issues in Brandon's.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1020_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1020.dat",
            27800,
            HyoutaUtils::Hash::SHA1FromHexString("5574310597160a8b94e2a8ccf2ad2dfdc22c79d2"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // formatting issues in Laura Chapter 3 Day bonding event
        std::swap(bin[0x575c], bin[0x5762]);
        std::swap(bin[0x5d6b], bin[0x5d6f]);
        std::swap(bin[0x60aa], bin[0x60ae]);

        // formatting issues in Jusis Chapter 5 Day bonding event
        std::swap(bin[0x49fa], bin[0x49fe]);
        std::swap(bin[0x4a21], bin[0x4a25]);
        std::swap(bin[0x4b9f], bin[0x4ba2]);
        std::swap(bin[0x4c95], bin[0x4c9a]);
        WriteUInt8(&bin[0x4cbc], 0x01);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1020_dat

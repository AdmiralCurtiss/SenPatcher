#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4610[] = "Typo fix in Gwyn's cabin.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t4610_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t4610.dat",
            9899,
            HyoutaUtils::Hash::SHA1FromHexString("ee2a5c698325223ef9aaf196af96e4a55dd18f16"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;

        // typo fix, Looks what way -> Looks that way
        WriteUInt8(&bin[0x6e5], 0x74);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t4610_dat

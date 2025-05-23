#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1110[] = "Fix formatting issues in Trista chapel.";
}

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1110_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1110.dat",
            59708,
            HyoutaUtils::Hash::SHA1FromHexString("29c1901bfb4050a4c0f62af6129c653883f4352c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // formatting issues in Gaius Chapter 3 Evening bonding event
        std::swap(bin[0xbb1b], bin[0xbb1f]);
        std::swap(bin[0xbc02], bin[0xbc06]);
        std::swap(bin[0xbc73], bin[0xbc78]);
        std::swap(bin[0xbd92], bin[0xbd97]);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t1110_dat

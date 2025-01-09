#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_r1010[] =
    "Fix missing word in Nicholas/Emily recruitment sidequest.";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_r1010_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/r1010.dat",
            29297,
            HyoutaUtils::Hash::SHA1FromHexString("c21cb1a876196b7551f3bc3ef57620c1ffc0deb1"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // missing 'it'
        std::swap(bin[0x4907], bin[0x490a]);
        patcher.ExtendPartialCommand(0x48dd, 0x39, 0x4904, {{0x69, 0x74, 0x20}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_r1010_dat

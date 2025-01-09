#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
// the message in insa09 suffices
//__declspec(dllexport) char SenPatcherFix_1_r3090[] =
//    "Change name to fixed FMV for second Chapter 3 map travel sequence.";
}

namespace SenLib::Sen3::FileFixes::r3090_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/r3090.dat",
            199489,
            HyoutaUtils::Hash::SHA1FromHexString("4e774a1dded6b6054525768a908fa5fd6b03ad25"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        bin[0x4926] = 0x66; // insa_09 -> insa_f9

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::r3090_dat

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m0600[] = "Capitalization fixes in Isthmia.";
}

namespace SenLib::Sen3::FileFixes::m0600_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m0600.dat",
            49985,
            HyoutaUtils::Hash::SHA1FromHexString("09161c59396b4aa8932f628fa9709ca8899fe58c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Time/Space/Mirage -> time/space/mirage
        bin[0x2de5] = 0x74;
        bin[0x2deb] = 0x73;
        bin[0x2df6] = 0x6d;
        bin[0x2fb3] = 0x74;
        bin[0x2fb9] = 0x73;
        bin[0x2fc4] = 0x6d;
        bin[0xb1e6] = 0x74;
        bin[0xb1ec] = 0x73;
        bin[0xb1f7] = 0x6d;
        bin[0xb265] = 0x74;
        bin[0xb26b] = 0x73;
        bin[0xb276] = 0x6d;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m0600_dat

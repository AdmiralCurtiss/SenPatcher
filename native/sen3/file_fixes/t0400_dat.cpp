#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0400[] = "Text fixes in Einhel Keep (Outside).";
}

namespace SenLib::Sen3::FileFixes::t0400_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0400.dat",
            37289,
            HyoutaUtils::Hash::SHA1FromHexString("05ebfadeed23981b39da835f2eb179877003492a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        using namespace HyoutaUtils::Vector;

        // grandpa -> old man
        // This is an editing error. The previous line (from Millium) said Grandpa Schmidt on PS4
        // disc but got changed to Old Man Schmidt in patch 1.02. The follow-up line from Schmidt to
        // stop calling him that did not get changed, however, leading to a weird mismatch.
        WriteAt(bin, 0x4e07, {{0x6f, 0x6c, 0x64, 0x20, 0x6d, 0x61, 0x6e}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0400_dat

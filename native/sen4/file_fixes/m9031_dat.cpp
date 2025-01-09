#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9031[] = "Newline fix in final dungeon.";
}

namespace SenLib::Sen4::FileFixes::m9031_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m9031.dat",
            81313,
            HyoutaUtils::Hash::SHA1FromHexString("cdc0b7a54ba0420a0c7c314d2a021f6949f5cb2e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        std::swap(bin[0x7d13], bin[0x7d1d]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen4::FileFixes::m9031_dat

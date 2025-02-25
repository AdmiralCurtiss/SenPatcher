#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_a0417[] = "Fix capitalization in Chapter 1 Ex. Camp";
}

namespace SenLib::Sen3::FileFixes::a0417_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    auto file = getCheckedFile(
        "data/scripts/scena/dat_en/a0417.dat",
        112657,
        HyoutaUtils::Hash::SHA1FromHexString("2b0b345b51b705b71ddc5f7989836dc3139f62f5"));
    if (!file) {
        return false;
    }

    auto& bin = file->Data;

    // Imperial government -> Imperial Government
    bin[0xfe3b] = 0x47;

    result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

    return true;
}
} // namespace SenLib::Sen3::FileFixes::a0417_dat

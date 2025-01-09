#include <array>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_2_alchr022[] = "Fix Rufus' Sword name";
}

namespace SenLib::Sen3::FileFixes::alchr022_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    auto file = getCheckedFile(
        "data/scripts/battle/dat_en/alchr022.dat",
        2341,
        HyoutaUtils::Hash::SHA1FromHexString("2c6088c4f1e8847e49f5f5f48b410fe2aec3ef54"));
    if (!file) {
        return false;
    }

    auto& bin = file->Data;

    // Holy Sword Ishnard -> Holy Sword of Ishnard
    std::array<char, 0x18> data;
    std::memcpy(data.data(), &bin[0x384], data.size());
    std::vector<char> newdata;
    newdata.insert(newdata.end(), data.begin(), data.begin() + 0xb);
    newdata.push_back(0x6f);
    newdata.push_back(0x66);
    newdata.insert(newdata.end(), data.begin() + 0xa, data.begin() + 0xa + 8);
    std::memcpy(&bin[0x384], newdata.data(), newdata.size());

    result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

    return true;
}
} // namespace SenLib::Sen3::FileFixes::alchr022_dat

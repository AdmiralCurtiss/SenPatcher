#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen3::FileFixes::book07_dat {
std::string_view GetDescription() {
    return "Fix typos in Black Records.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_en/book07.dat",
            34151,
            HyoutaUtils::Hash::SHA1FromHexString("4531fd2f226f5c641e09fc35e118d1c1bb2a4144"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // great knight -> Great Knight
        bin[0x40a1] = 0x47;
        bin[0x40a7] = 0x4b;

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::book07_dat

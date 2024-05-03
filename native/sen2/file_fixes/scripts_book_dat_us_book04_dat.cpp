#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book04_dat {
std::string_view GetDescription() {
    return "Fix translation error in headline of Imperial Chronicle Issue 3.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book04.dat",
            51414,
            HyoutaUtils::Hash::SHA1FromHexString("5b2fee612159bcb93b2c6831f94f7b1f4dd6231c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // translation error in last headline of imperial chronicle 3
        HyoutaUtils::MemWrite::WriteArray(&bin[0x4124], {{0x45, 0x61}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book04_dat

#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_thomas_dat {
std::string_view GetDescription() {
    return "Fix typo in conversation with Thomas.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_us/tk_thomas.dat",
            29722,
            SenPatcher::SHA1FromHexString("465873e53df8cb85e26d302a171eb4cc1ff2d6bb"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Byronia -> Bryonia
        std::swap(bin[0x54fd], bin[0x54fe]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_talk_dat_us_tk_thomas_dat

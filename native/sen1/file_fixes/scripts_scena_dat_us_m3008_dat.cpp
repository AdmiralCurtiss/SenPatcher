#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sha1.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m3008_dat {
std::string_view GetDescription() {
    return "Fix typo in final dungeon.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/m3008.dat",
            51513,
            SenPatcher::SHA1FromHexString("f44b397573f1127711f66ec631b2de5dffdafc38"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // I'm Glad -> I'm glad
        bin[0x97c4] = static_cast<char>(0x67);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_m3008_dat

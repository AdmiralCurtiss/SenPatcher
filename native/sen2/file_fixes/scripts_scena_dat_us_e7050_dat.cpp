#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7050_dat {
std::string_view GetDescription() {
    return "Fix typo in Grianos' name.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/e7050.dat",
            126953,
            HyoutaUtils::Hash::SHA1FromHexString("d0a9a39e3aa04d573b9a7b51a170fb1dc4d79f17"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);

        // Girianos -> Grianos
        patcher.RemovePartialCommand(0x9369, 0xbb, 0x93de, 0x1);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7050_dat

#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3500_dat {
std::string_view GetDescription() {
    return "Text/Voice match in Roer.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t3500.dat",
            156209,
            SenPatcher::SHA1FromHexString("3f981082b2affc869785a08896aad11a50c44343"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        // Huh? -> Hmm?
        // This is in Act 2, early into the Roer section, right before reuniting with Angelica.
        HyoutaUtils::MemWrite::WriteArray(&bin[0x17e54], {{0x6d, 0x6d}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t3500_dat

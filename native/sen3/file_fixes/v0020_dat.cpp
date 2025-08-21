#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_v0020[] = "Fix grammar in Derfflinger Car 3.";
}

namespace SenLib::Sen3::FileFixes::v0020_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/v0020.dat",
            176369,
            HyoutaUtils::Hash::SHA1FromHexString("eeead345101968bb959d0efd14bfcc30f4997dc9"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#2PMight not seem like it sometimes, but I've got\x01three or four years worth of wisdom
        // on you."
        // years worth -> years' worth
        patcher.ExtendPartialCommand(0xd925, 0x66, 0xd971, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::v0020_dat

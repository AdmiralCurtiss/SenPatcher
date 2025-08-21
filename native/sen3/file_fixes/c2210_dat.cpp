#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2210[] =
    "Fix name consistency in Heimdallr (Plaza Bifrost)";
}

namespace SenLib::Sen3::FileFixes::c2210_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2210.dat",
            75601,
            HyoutaUtils::Hash::SHA1FromHexString("3df00eb7c90560622af736506fb5dcc902cb367f"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "'The Antique Art Dealers Association,' hm?\x01I've heard of it before."
        // name should be "Dealers'" instead of "Dealers"
        // (chapter 4, 7/16, during the Expensive Delivery quest, talk to Lepanto after delivering
        // his package)
        patcher.ExtendPartialCommand(0x6f82, 0xb0, 0x6FA1, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2210_dat

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1020[] = "Fix Elliot's name in Saint-Arkh chapel";
}

namespace SenLib::Sen3::FileFixes::t1020_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t1020.dat",
            51473,
            HyoutaUtils::Hash::SHA1FromHexString("e5b0cb4a35f6450358f97d6548ec13256b699255"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // Elliott -> Elliot
        // during the scene where he's introduced (EV_01_21_00)
        patcher.RemovePartialCommand(0x5a4b, 0x4b, 0x5a5d, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t1020_dat


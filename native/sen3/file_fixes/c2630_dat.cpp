#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2630[] =
    "Fix typo in Heimdallr (Hershel's General Store)";
}

namespace SenLib::Sen3::FileFixes::c2630_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2630.dat",
            39457,
            HyoutaUtils::Hash::SHA1FromHexString("eb063b555d29ccb888e612c4188d6af56355be05"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "As long as I'm here, I won't allow you to\x01do anything weirdo, you creepo!"
        // weirdo -> weird
        // (chapter 4, 7/15 night, right before the class 7 meet at the guild, NPC dialogue with
        // Kai)
        patcher.RemovePartialCommand(0xeca, 0x52, 0xf0c, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2630_dat

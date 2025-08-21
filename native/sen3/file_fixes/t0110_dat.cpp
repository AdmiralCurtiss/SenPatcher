#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0110[] = "Fix typo in Leeves (Private Home)";
}

namespace SenLib::Sen3::FileFixes::t0110_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0110.dat",
            13185,
            HyoutaUtils::Hash::SHA1FromHexString("b38377b703b17721ee335d6dd7d0e544f7a1fb29"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Oh, wow! What a feast! \x01What's the occassion?"
        // occassion -> occasion
        // (chapter 4, 7/9 afternoon, NPC dialogue with Braun)
        patcher.RemovePartialCommand(0x2035, 0x36, 0x2064, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0110_dat

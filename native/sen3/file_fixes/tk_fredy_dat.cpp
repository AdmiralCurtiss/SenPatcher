#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_freddy[] = "Fix typo in conversation with Freddy.";
}

namespace SenLib::Sen3::FileFixes::tk_fredy_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_fredy.dat",
            32593,
            HyoutaUtils::Hash::SHA1FromHexString("2bf44e8191fcb09741ab76eafa1bad1de71fdff2"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_4#M_AHahaha! I can't wait till its done!"
        // its done -> it's done
        // (chapter 1, 4/16 morning, dorm 1F)
        patcher.ExtendPartialCommand(0x2be3, 0xdd, 0x2cb7, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_fredy_dat

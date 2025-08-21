#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_kurt[] = "Fix typo in conversation with Kurt.";
}

namespace SenLib::Sen3::FileFixes::tk_kurt_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_kurt.dat",
            18209,
            HyoutaUtils::Hash::SHA1FromHexString("b00ca82710009006f58532f5b932ae5569ca4f09"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#k#1U(I mean... I could, but if I say something\x01now, it might just makes things
        // worse.)"
        // makes things -> make things
        // (chapter 4, 7/17 free time, Dreichel's Plaza)
        patcher.RemovePartialCommand(0x368, 0x8f, 0x3b6, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_kurt_dat

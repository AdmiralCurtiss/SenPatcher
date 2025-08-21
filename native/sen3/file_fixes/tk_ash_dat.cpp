#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_ash[] =
    "Fix spelling consistency in conversation with Ash.";
}

namespace SenLib::Sen3::FileFixes::tk_ash_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_ash.dat",
            24577,
            HyoutaUtils::Hash::SHA1FromHexString("5f9dedd2fc457edb33a9d985c0c47bc481cdf00d"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#kHeh, you're one popular guy,\x01aint'cha, Mister Hero?"
        // aint'cha -> ain'tcha
        // (chapter 2, 5/14 afternoon, school roof)
        // patcher.ReplacePartialCommand(0x3952, 0x83, 0x3959, 0x35, "");
        std::swap(bin[0x397b], bin[0x397c]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_ash_dat

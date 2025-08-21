#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2840[] =
    "Fix typography in Heimdallr (Vander Training Hall)";
}

namespace SenLib::Sen3::FileFixes::c2840_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2840.dat",
            59529,
            HyoutaUtils::Hash::SHA1FromHexString("3992be1c03e58ab875f051fe14f53c1372f4d668"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#KThis is...a master quartz!?"
        // !? -> ?!
        // patcher.ReplacePartialCommand(0xdec4, 0x46, 0xdecb, 0x1d, "");
        // (after beating Aurier in Vantage Master, must have beaten everyone else first. chapter 4,
        // 7/17 free time is the earliest (only?) opportunity in a regular playthrough)
        std::swap(bin[0xdee6], bin[0xdee7]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2840_dat

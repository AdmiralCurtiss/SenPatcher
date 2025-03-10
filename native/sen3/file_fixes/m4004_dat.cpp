#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m4004[] =
    "Fix terminology consistency issues in Gral of Erebos (Bottom).";
}

namespace SenLib::Sen3::FileFixes::m4004_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/m4004.dat",
            231153,
            HyoutaUtils::Hash::SHA1FromHexString("5941f93f60a4ab76661ae37e51103b8f1cf35011"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // Sept-Terrion of Flame -> Sept-Terrion of Fire
        patcher.ReplacePartialCommand(0x33b7, 0x82, 0x33e3, 0x3, {{0x69, 0x72}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::m4004_dat

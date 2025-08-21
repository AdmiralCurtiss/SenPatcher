#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2810[] =
    "Fix typography error in Heimdallr (Lucien Art Cafe)";
}

namespace SenLib::Sen3::FileFixes::c2810_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2810.dat",
            22961,
            HyoutaUtils::Hash::SHA1FromHexString("09f7e21f6882a2c0a9e26775438c5603f77b4140"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E[5]#M_A#B_0That’s some dedication\x01they've got, huh?"
        // wrong apostrophe
        // (chapter 4, 7/16 morning, NPC dialogue with Nancy)
        patcher.ReplacePartialCommand(0x232d, 0xcf, 0x23d4, 3, {{'\''}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2810_dat

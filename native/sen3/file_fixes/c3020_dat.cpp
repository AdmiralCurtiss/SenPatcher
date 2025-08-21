#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c3020[] = "Fix text errors in Heimdallr (Hotel Valar)";
}

namespace SenLib::Sen3::FileFixes::c3020_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c3020.dat",
            56449,
            HyoutaUtils::Hash::SHA1FromHexString("7fb4e1457ba955aa3659f851603592aa20dcbbff"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Hello there, everyone." (Sharon)
        // It's just Rean in this scene without a party, so 'everyone' is wrong.
        // Change to 'Master Rean'.
        // (chapter 4, 7/17 free time, scene with Sharon and Irina)
        patcher.ReplacePartialCommand(0x7cbd, 0x1f, 0x7cd1, 8, STR_SPAN("Master Rean"));

        // " This is a matter of national import. We'll\x01have to work together to make it
        // through."
        // extra space
        // (final chapter, NPC dialogue with Manager Sheldon)
        patcher.RemovePartialCommand(0x1cf9, 0xf7, 0x1d99, 1);

        // "Rean... Musse... Everyone...\x01Thank goodness you're all okay.."
        // two dots -> three dots
        // (final chapter, scene with Elise/Irina/Gwyn)
        patcher.ExtendPartialCommand(0x953b, 0x85, 0x957d, {{'.'}});

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c3020_dat

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

extern "C" {
__declspec(dllexport) char SenPatcherFix_2_ztk_common[] = "";
}

namespace SenLib::Sen3::FileFixes::tk_common_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_common.dat",
            243353,
            HyoutaUtils::Hash::SHA1FromHexString("a1df4407306647ca393a7ba32f5a7b41f435190c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_2#M_2#B_0Going up against someone like\x01you should be a good experience\x01for the
        // kids.."
        // extra dot
        // (chapter 4, 7/15, right after arriving at the exercise camp, talk to Randy/Aurelia)
        patcher.RemovePartialCommand(0x3393c, 0xe1, 0x33a1a, 1);

        // "#E_0#M_2#B_0Though they may be the respective\x01heads of the of the Noble
        // and\x01Reformist Factions..."
        // double 'of the'
        // we're also reformatting this while we're here...
        // (final chapter, outside the Heimdallr horse race course)
        bin[0xcb2a] = '\x01';
        std::swap(bin[0xcb89], bin[0xcb93]);
        patcher.RemovePartialCommand(0xcb06, 0xd9, 0xcb79, 7);

        // "#KYes. I'll be sure tell you about it\x01at some point."
        // sure tell -> sure to tell
        // (chapter 4, esmelas garden party, talk to Gaius in first set)
        patcher.ExtendPartialCommand(0x107ed, 0x3d, 0x10808, STR_SPAN("to "));

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_common_dat

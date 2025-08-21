#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_jingo[] =
    "Fix spelling consistency in conversation with Jingo.";
}

namespace SenLib::Sen3::FileFixes::tk_jingo_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/talk/dat_en/tk_jingo.dat",
            24601,
            HyoutaUtils::Hash::SHA1FromHexString("41f45fa083b8624f81692e9253618ee80041f398"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "Ain't'cha playin' tag with those Calvardians,\x01or somethin'? How's that goin'?"
        // Ain't'cha -> Ain'tcha for consistency
        // (chapter 4, 7/16 morning, in Watson's Weapons & Armor)
        patcher.RemovePartialCommand(0x841, 0x7b, 0x872, 1);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::tk_jingo_dat

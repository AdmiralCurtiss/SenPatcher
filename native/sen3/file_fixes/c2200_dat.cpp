#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_c2200[] =
    "Fix typography issues in Heimdallr (Vainqueur Street)";
}

namespace SenLib::Sen3::FileFixes::c2200_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/c2200.dat",
            217737,
            HyoutaUtils::Hash::SHA1FromHexString("bc7bd4e735660daf5196032dcae1d4515b420f13"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#E_8#M_A#B_0It’d be bad if I got overwhelmed by the\x01big picture and let all the
        // important\x01stuff slip through the cracks."
        // wrong apostrophe
        // (towa final bond event scene after festival ticket)
        patcher.ReplacePartialCommand(0x30931, 0xc0, 0x30983, 3, {{'\''}});

        // "Wh-What's going on!?"
        // !? -> ?!
        // patcher.ReplacePartialCommand(0xaa3c, 0x84, 0xaa43, 0x14, "");
        // (chapter 4, 7/15, NPC truck driver during the Riding the Orbal Waves quest)
        std::swap(bin[0xAA55], bin[0xAA56]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::c2200_dat

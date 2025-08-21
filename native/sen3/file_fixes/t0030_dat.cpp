#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0030[] = "Fix typography in Leeves (church)";
}

namespace SenLib::Sen3::FileFixes::t0030_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0030.dat",
            75553,
            HyoutaUtils::Hash::SHA1FromHexString("e7137edd7afd74039f42b7215c129a899727cb8e"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // "#KSt. Astraia!?\x01That's that super famous one...!"
        // !? -> ?!
        // this line seems poorly phrased in general but oh well...
        // (chapter 3, 6/11 morning, during the A Very Special Lession sidequest)
        // patcher.ReplacePartialCommand(0xa660, 0x39, 0xa667, 0x30, "");
        std::swap(bin[0xa674], bin[0xa675]);

        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0030_dat

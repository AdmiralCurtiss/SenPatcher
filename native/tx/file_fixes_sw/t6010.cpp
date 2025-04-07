#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t6010[] = "Text fixes in Houraichou Back Alley.";
}

namespace SenLib::TX::FileFixesSw::t6010 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6010.dat",
            43065,
            HyoutaUtils::Hash::SHA1FromHexString("aecd6624258896072ac1d1c273876e9fddd95c8b"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "As a precaution, Kou contacted Asuka and Sora via NiAR."
        // This is wrong, you can be either with Sora or with Yuuki for this scene and the
        // other one is with Asuka. We'll just drop the "and Sora" part.
        // (EV_03_10_01)
        patcher.RemovePartialCommand(0x1b27, 0x3d, 0x1B50, 9);

        // "#2P#4S#3C#F#800W#3CHeeheehee...#10W...\x01#6S#1000WAHAHAHA!"
        // drop '#10W...'
        // (EV_03_11_01)
        patcher.RemovePartialCommand(0x4ebb, 0x72, 0x4EE2, 7);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6010

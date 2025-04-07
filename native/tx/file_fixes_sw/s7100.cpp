#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s7100[] = "Text fixes in hot springs lobby.";
}

namespace SenLib::TX::FileFixesSw::s7100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s7100.dat",
            55225,
            HyoutaUtils::Hash::SHA1FromHexString("759c2f169ff07032e1bc3d4f392d0bd191328296"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Everything but my preformance\x01at work, that is..."
        // preformance -> performance
        // (NPC dialogue with Suzuna and Kasumi after the big discussion in the Intermission)
        // patcher.ReplacePartialCommand(0x1ee8, 0x5f, 0x1eeb, 0x5a, "");
        std::swap(bin[0x1F27], bin[0x1F28]);

        // "#2K(Dinner's on the hall in the second\x01floor.)"
        // -> "Dinner's in the hall on the second floor"
        // (EV_05_04_00)
        // patcher.ReplacePartialCommand(0x606c, 0x7e, 0x606f, 0x2e, "");
        std::swap(bin[0x607C], bin[0x6088]);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s7100

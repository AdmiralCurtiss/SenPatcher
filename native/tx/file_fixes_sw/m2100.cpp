#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m2100[] = "Text fixes in Moonlit Garden.";
}

namespace SenLib::TX::FileFixesSw::m2100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m2100.dat",
            17609,
            HyoutaUtils::Hash::SHA1FromHexString("6d8feefd93f5d486128faaf6ca40c68b95a7d674"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);


        // "Ranged skills are for attacking from a distance and are effective against foes
        // #3Cresistant to physical attacks#1C."
        // missing newline
        // patcher.ReplacePartialCommand(0x3936, 0x26c, 0x3a03, 0x4, "");
        bin[0x39CB] = 0x1;

        // TODO: The button prompt here is wrong for PC.
        // "#3CPress #3C#915I to use it#1C. Each character's EX Skill is different,\x01so feel free
        // to use whichever you feel will work best."
        // patcher.ReplacePartialCommand(0x2f7e, 0xcf, 0x2fe8, 0x63, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m2100

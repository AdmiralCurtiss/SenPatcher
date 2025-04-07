#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m6309[] = "Text fixes in Rion awakening cutscene.";
}

namespace SenLib::TX::FileFixesSw::m6309 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m6309.dat",
            46385,
            HyoutaUtils::Hash::SHA1FromHexString("00b6b2f306071683f4cbf6497b49f71b6ae40a35"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Rion's Voice" -> "Haruna's Voice"
        // Fixed in Switch v1.0.1

        // "#1P#800W#5SPlease#50W… #1000WSave me!"
        // slow print only last character of ellipsis instead of all three
        // This would be good if the ellipsis looked identical to the three dots, but it doesn't,
        // so...
        // (EV_06_19_00)
        // patcher.ReplacePartialCommand(0x299d, 0x32, 0x29be, 0xe, "");
        bin[0x29B7] = '3';
        bin[0x29BA] = '.';
        bin[0x29BB] = '.';
        bin[0x29BC] = '.';
        patcher.ShiftData(0x29B6, 0x29BC, 4);

        // "#800WIf I'd known, I...\x01#6S#20W#800WI would've prayed for death instead!"
        // harmless
        // (EV_06_19_00)
        // patcher.ReplacePartialCommand(0x56a9, 0x52, 0x56d0, 0x29, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m6309

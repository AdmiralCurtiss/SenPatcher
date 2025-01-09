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
            HyoutaUtils::Hash::SHA1FromHexString("2de6acb9d74023d7411490661362d530c0b609b9"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Rion's Voice" -> "Haruna's Voice"
        // incorrect name tag on the textbox
        patcher.ReplacePartialCommand(
            0x174c, 0x48, 0x1757, 0x4, {{0x48, 0x61, 0x72, 0x75, 0x6e, 0x61}});

        // "#1P#800W#5SPlease#50W...#1000WSave me!"
        // slow print only last character of ellipsis instead of all three
        patcher.ShiftData(0x29b4, 0x29ba, 4);
        bin[0x29b7] = 0x38;
        patcher.RemovePartialCommand(0x299b, 0x31, 0x29b8, 0x1);

        // "#800WIf I'd known, I...\x01#6S#20W#800WI would've prayed for death instead!"
        // patcher.ReplacePartialCommand(0x56a6, 0x52, 0x56cd, 0x29, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m6309

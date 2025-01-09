#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s8000[] = "Text fixes in NDF base.";
}

namespace SenLib::TX::FileFixesSw::s8000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s8000.dat",
            187057,
            HyoutaUtils::Hash::SHA1FromHexString("864903ca30fd6359323cfd9d68f1a2ae688e2b25"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "But you're a kid." -> "But you're kids."
        // the entire XRC is standing there and is saying 'we're gonna do this!!',
        // it's very weird for him to only address one
        patcher.ShiftData(0x8b9b, 0x8ba0, 2);
        patcher.ReplacePartialCommand(0x8b85, 0x1e, 0x8b9e, 0x2, {{0x73}});

        // "#2P#7C#800W#7CIt's about the traces of twilight.#18W\x01#1000WThey're still here."
        // patcher.ReplacePartialCommand(0x10510, 0x58, 0x1054d, 0x19, "");

        // "#E_F#M_A#2P#800WAre you... #10W #800WAre you at Acros Tower?"
        // patcher.ReplacePartialCommand(0x11099, 0x46, 0x110c1, 0x1c, "");

        // "#2C#0T#200W#2CHurry. #18W...#500WGet out#20W,#5S#800Wof the city...!"
        // patcher.ReplacePartialCommand(0x1115f, 0x4f, 0x11197, 0x14, "");

        // "#2P#5S#1C#0T#800W#1CBlade of perpetual ice!\x01#23W#1000WIcicle Nova!"
        // patcher.ReplacePartialCommand(0x1c1b4, 0x4d, 0x1c1ec, 0x12, "");

        // "#E[7]#M_A#2K#FI can't fight anymore.\x01#1000W#5SRun, Shiori!"
        // patcher.ReplacePartialCommand(0x1e48e, 0x44, 0x1e4c1, 0xf, "");

        // "#2P#3C#600W#3CHow about you come play\x01with me instead, Shiori?"
        // patcher.ReplacePartialCommand(0x21662, 0x48, 0x21675, 0x33, "");

        // "#0T#1C#1CSearch complete#16W...#1000W\x01No results found."
        // patcher.ReplacePartialCommand(0x253f8, 0x41, 0x2541f, 0x18, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s8000

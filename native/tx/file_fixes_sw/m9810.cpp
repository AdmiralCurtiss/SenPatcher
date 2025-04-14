#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m9810[] = "Text fixes in " DUNGEON_NAME_m9810 ".";
}

namespace SenLib::TX::FileFixesSw::m9810 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9810.dat",
            33257,
            HyoutaUtils::Hash::SHA1FromHexString("ed1d437c3ad5f27d55252a31300db0548469f49b"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[M]#M_B#e[B]#5S#0T#FDon't you lay a finger on her,\x01#1000W#6SASSHOLE!"
        // harmless
        // (EV_17_11_00)
        // patcher.ReplacePartialCommand(0x4b47, 0x51, 0x4b8a, 0xb, "");

        // "#K#F#5SI won't back down, either!"
        // this doesn't even have a W tag anymore...
        // (EV_17_11_00)
        // patcher.ReplacePartialCommand(0x52d0, 0x2b, 0x52dc, 0x1d, "");

        // "#2PWe will surely win with everyone here!\x01#8W#1000W#5SCome on!"
        // harmless
        // (EV_17_11_00)
        // patcher.ReplacePartialCommand(0x5df1, 0x48, 0x5e2c, 0xb, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9810

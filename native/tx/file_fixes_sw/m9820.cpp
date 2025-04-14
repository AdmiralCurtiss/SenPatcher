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
__declspec(dllexport) char SenPatcherFix_1_m9820[] = "Text fixes in " DUNGEON_NAME_m9820 ".";
}

namespace SenLib::TX::FileFixesSw::m9820 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9820.dat",
            39081,
            HyoutaUtils::Hash::SHA1FromHexString("9c6b9e22d06c8f4c19ad9383019a552cffd6d857"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#0TGot it#20W!\x01#1000W#5SNOW!"
        // fine as-is
        // (EV_17_15_00)
        // patcher.ReplacePartialCommand(0x4818, 0x27, 0x4835, 0x7, "");

        // "#1PDamn it! No other way around it...\x01#1000W#5SStand back, Hokuto!"
        // harmless
        // (EV_17_15_00)
        // patcher.ReplacePartialCommand(0x61ff, 0x4c, 0x6233, 0x16, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9820

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

namespace SenLib::TX::FileFixesSw::m3800 {
std::string_view GetDescription() {
    return "Text fixes in " DUNGEON_NAME_m3800 ".";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m3800.dat",
            16441,
            HyoutaUtils::Hash::SHA1FromHexString("e16c2829ab07499b91855356c0c2c0b61ba9b061"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E[11111111111111111111116]#M_AWithout Tokisaka and the rest here...\x01#9W#1000WThere's
        // no need to hold back."
        // patcher.ReplacePartialCommand(0x24d1, 0x75, 0x2521, 0x23, "");

        // "#2P#2C#5S#2CFirst binding spell, release!"
        // patcher.ReplacePartialCommand(0x29aa, 0x34, 0x29bb, 0x20, "");

        // "#8P#F#2C#5S#F#0T#2CThis is Asuka Hiiragi, the\x01Astral Blade. #10W#1000WCommencing
        // mission!"
        // patcher.ReplacePartialCommand(0x3287, 0x63, 0x32cf, 0x19, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m3800

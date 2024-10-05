#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#include "dungeon_names.h"

namespace SenLib::TX::FileFixesSw::m9830 {
std::string_view GetDescription() {
    return "Text fixes in " DUNGEON_NAME_m9830 ".";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9830.dat",
            58329,
            HyoutaUtils::Hash::SHA1FromHexString("96117602af3df79cc1e8e65a5b85745c1766e646"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#5S#0TLet me finish this real quick.\x01#12W#1000WArc Trinity!"
        // patcher.ReplacePartialCommand(0x4b4b, 0x46, 0x4b7c, 0x12, "");

        // "#1P#5S#3C#600W#3CConsidering...#20WI died\x01#300WBECAUSE OF YOU!!!"
        // this one is even more broken than usual, the line doesn't even get to fully finish before
        // the game advances the textbox...
        // patcher.ReplacePartialCommand(0x9b94, 0x4b, 0x9bc6, 0x16, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9830

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::m8390 {
std::string_view GetDescription() {
    return "Text fixes in Pandora (final boss room).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8390.dat",
            49601,
            HyoutaUtils::Hash::SHA1FromHexString("3abea04b7a0c555b8b5367810a365766f84849c3"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E_6#M_0#2P#800WShiori's been there with us this\x01whole time. And I won't let
        // you\x01deny it any longer!"
        // patcher.ReplacePartialCommand(0x72b1, 0x6e, 0x72c4, 0x59, "");

        // "#1P#800WEveryone#15W...#1000W#5SLend me your strength!"
        // patcher.ReplacePartialCommand(0x88b0, 0x40, 0x88d5, 0x19, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8390

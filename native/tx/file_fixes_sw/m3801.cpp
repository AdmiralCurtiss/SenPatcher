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
__declspec(dllexport) char SenPatcherFix_1_m3801[] = "Text fixes in " DUNGEON_NAME_m3800
                                                     " (boss room).";
}

namespace SenLib::TX::FileFixesSw::m3801 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m3801.dat",
            9537,
            HyoutaUtils::Hash::SHA1FromHexString("451fb8e957b4d92a59b9e6c4c81be7c12e283e32"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1P#2C#F#2CThe labyrinth master!\x01#18W#1000W#5SYou fall here!"
        // patcher.ReplacePartialCommand(0x164d, 0x46, 0x1680, 0x11, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m3801

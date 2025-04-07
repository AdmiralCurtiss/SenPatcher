#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m2090[] = "Text fixes in Amber Labyrinth (boss room).";
}

namespace SenLib::TX::FileFixesSw::m2090 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m2090.dat",
            13345,
            HyoutaUtils::Hash::SHA1FromHexString("514bb70e4ecf17bad5f14a9077a61b84fe0401ca"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);


        // TODO: fix button mappings for PC:
        // #1C#1CGo to the #170I mini map screen and press #168I to warp.
        // patcher.ReplacePartialCommand(0x2a15, 0x103, 0x2b08, 0xe, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m2090

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m8390[] = "Text fixes in Pandora (final boss room).";
}

namespace SenLib::TX::FileFixesSw::m8390 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8390.dat",
            49601,
            HyoutaUtils::Hash::SHA1FromHexString("44d93d6a90252c57cc1e84dcf51b6a1b41a0b6e2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E_6#M_0#2P#800WShiori's been there with us this\x01whole time. And I won't let
        // you\x01deny it any longer!"
        // harmless
        // (EV_08_27_01)
        // patcher.ReplacePartialCommand(0x72a6, 0x6e, 0x72b9, 0x59, "");

        // "#1P#800WEveryone…\x01#1000W#5SLend me your strength!"
        // tags are harmless, but the ellipsis needs to be dots
        // (EV_08_27_01)
        // patcher.ReplacePartialCommand(0x88a5, 0x3d, 0x88c7, 0x19, "");
        bin[0x88BD] = '.';
        bin[0x88BE] = '.';
        bin[0x88BF] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8390

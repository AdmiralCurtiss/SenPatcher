#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1001[] = "Text fixes outside school (Mist Castle).";
}

namespace SenLib::TX::FileFixesSw::t1001 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1001.dat",
            18145,
            HyoutaUtils::Hash::SHA1FromHexString("e4ba0823b6818d1b93794b99ccba9df0ce2d028b"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Kou and his friends bid their farewells to Mayu, Yamaoka, and Mizuhara\x01
        // before setting off."
        // escapes textbox, move linebreak
        // patcher.ReplacePartialCommand(0x32c0, 0x60, 0x32c3, 0x5b, "");
        std::swap(bin[0x32FD], bin[0x330A]);

        // "#800WI can't get in contact with Shiori#15W,\x01#800WRyouta, Jun, or
        // Towa.\x01#10W#5S#800WDamn it!"
        // broken text speed, harmless
        // patcher.ReplacePartialCommand(0x236a, 0x66, 0x23c1, 0xd, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1001

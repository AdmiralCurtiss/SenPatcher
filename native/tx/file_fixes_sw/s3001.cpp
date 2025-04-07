#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s3001[] = "Text fixes outside Pandora.";
}

namespace SenLib::TX::FileFixesSw::s3001 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s3001.dat",
            127033,
            HyoutaUtils::Hash::SHA1FromHexString("5e4eee3f8882232b3d635b6f631e9b1bb5b02a3f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K#0T#5S#800WGramps#20W?!\x01#1000W#4STowa and Ryouta, too?!"
        // shift the '#20W' between the '?!' so it works as intended
        // (EV_08_22_03)
        // patcher.ReplacePartialCommand(0xe152, 0x43, 0xe17a, 0x19, "");
        patcher.ShiftData(0xE171, 0xE16D, 1);

        // "#2C#5S#500W#2CFirst#40W, #500Wsecond#40W, #500Wand third binding spells... Release!"
        // surprisingly, this one's fine!
        // (EV_08_22_07)
        // patcher.ReplacePartialCommand(0x149c6, 0x5e, 0x149f8, 0x29, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3001

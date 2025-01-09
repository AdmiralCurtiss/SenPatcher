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
            127209,
            HyoutaUtils::Hash::SHA1FromHexString("922f8fa5f3ce32a952e06b17a493c5e3c6af58a8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K#0T#5S#800WGramps#20W?!\x01#1000W#4STowa and Ryouta, too?!"
        // patcher.ReplacePartialCommand(0xe1de, 0x43, 0xe206, 0x19, "");

        // "#2C#5S#500W#2CFirst#40W, #500Wsecond#40W, #500Wand third binding spells... Release!"
        // patcher.ReplacePartialCommand(0x14a7a, 0x5e, 0x14aac, 0x29, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s3001

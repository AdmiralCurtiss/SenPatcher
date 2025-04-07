#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s7130[] = "Text fixes in hot springs outdoor bath.";
}

namespace SenLib::TX::FileFixesSw::s7130 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s7130.dat",
            60369,
            HyoutaUtils::Hash::SHA1FromHexString("eb36a01754415e701e8cc72eca2cfa95d8573538"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#E_4#M_9Now that I think about it, I think\x01there are outdoor baths are in
        // the\x01detached annex, too."
        // there's an 'are' too much here
        // (EV_05_01_04)
        patcher.RemovePartialCommand(0x4865, 0x8b, 0x48CF, 4);

        // "#3C#0T#3S#3CDamn it, I can't reach#15W...\x01#1000W#5SWHOA!"
        // surprisingly okay already
        // (EV_05_01_04)
        // patcher.ReplacePartialCommand(0x447c, 0x43, 0x44b4, 0x8, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s7130

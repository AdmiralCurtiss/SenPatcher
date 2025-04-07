#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s2100[] = "Text fixes in Yuuki's Apartment.";
}

namespace SenLib::TX::FileFixesSw::s2100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s2100.dat",
            66953,
            HyoutaUtils::Hash::SHA1FromHexString("6ebf051b194133cb3e642b4d3cdba85b8c852e47"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Set distance from 'Point Q.'\x01No change for 'Awakening Phase.'"
        // this is a direct quote from the paper, which doesn't have the quotes anymore, so remove
        // them here too
        // (EV_07_20_02)
        patcher.ShiftData(0xb178, 0xb189, 1);
        patcher.ShiftData(0xb168, 0xb189, 1);
        patcher.ShiftData(0xb15f, 0xb189, 1);
        patcher.RemovePartialCommand(0xb100, 0x8f, 0xb186, 0x4);

        // "#5C#800W#5CNAME: Futaba Ichinose\x01#800WDIED: March 15, 2005#10W\x01#800WAGE: 18 years"
        // harmless
        // (EV_07_20_02)
        // patcher.ReplacePartialCommand(0xd1df, 0x56, 0xd221, 0x12, "");

        // "#2P＂#800WLong-distance relationship＂...#20W\x01#800WSo that's what it was."
        // harmless
        // (EV_07_20_02)
        // patcher.ReplacePartialCommand(0xd4df, 0x50, 0xd512, 0x1b, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2100

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
            HyoutaUtils::Hash::SHA1FromHexString("c80d4c5fae8916b7e4fc540dcfab9e6f56cdf189"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Set distance from 'Point Q.' No changes for 'Awakening Phase.'"
        // this is a direct quote from the paper, which doesn't have the quotes anymore, so remove
        // them here too
        patcher.ShiftData(0xb168, 0xb15f, 1);
        patcher.ShiftData(0xb179, 0xb15f, 1);
        patcher.ShiftData(0xb18a, 0xb15f, 1);
        patcher.RemovePartialCommand(0xb100, 0x90, 0xb15f, 0x4);

        // "#5C#800W#5CNAME: Futaba Ichinose\x01#800WDIED: March 15, 2005#10W\x01#800WAGE: 18 years"
        // patcher.ReplacePartialCommand(0xd1e1, 0x56, 0xd223, 0x12, "");

        // "#2P"#800WLong-distance relationship"...#20W\x01#800WSo that's what it was."
        // patcher.ReplacePartialCommand(0xd4e1, 0x50, 0xd514, 0x1b, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2100

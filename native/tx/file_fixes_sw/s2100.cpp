#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::s2100 {
std::string_view GetDescription() {
    return "Text fixes in Yuuki's Apartment.";
}

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


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2100

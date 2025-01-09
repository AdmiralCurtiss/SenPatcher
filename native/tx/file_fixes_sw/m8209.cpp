#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
// reunite with Rion/Mitsuki
__declspec(dllexport) char SenPatcherFix_1_m8209[] = "Text fixes in Lethe Pillar (midpoint).";
}

namespace SenLib::TX::FileFixesSw::m8209 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8209.dat",
            19241,
            HyoutaUtils::Hash::SHA1FromHexString("b9def7a2ac604b7a3d69949cb97045a930e69f0c"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#4S#6P#800WYou#50W...\x01#1000W#5SYou idiot!"
        // patcher.ReplacePartialCommand(0x2f22, 0x33, 0x2f46, 0xd, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8209

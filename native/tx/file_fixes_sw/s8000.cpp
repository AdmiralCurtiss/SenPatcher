#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::s8000 {
std::string_view GetDescription() {
    return "Text fixes in NDF base.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s8000.dat",
            187057,
            HyoutaUtils::Hash::SHA1FromHexString("864903ca30fd6359323cfd9d68f1a2ae688e2b25"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "But you're a kid." -> "But you're kids."
        // the entire XRC is standing there and is saying 'we're gonna do this!!',
        // it's very weird for him to only address one
        patcher.ShiftData(0x8b9b, 0x8ba0, 2);
        patcher.ReplacePartialCommand(0x8b85, 0x1e, 0x8b9e, 0x2, {{0x73}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s8000

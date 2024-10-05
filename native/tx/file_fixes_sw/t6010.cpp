#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t6010 {
std::string_view GetDescription() {
    return "Text fixes in Houraichou Back Alley.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6010.dat",
            43065,
            HyoutaUtils::Hash::SHA1FromHexString("4a896bbd0b91322dae88c7979225bf9d0c4fc6a2"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2P#4S#3C#F#800W#3CHeeheehee...#10W...\x01#6S#1000WAHAHAHA!"
        // patcher.ReplacePartialCommand(0x4ec3, 0x72, 0x4ef5, 0xe, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6010

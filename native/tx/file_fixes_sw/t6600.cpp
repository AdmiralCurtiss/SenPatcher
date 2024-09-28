#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::t6600 {
std::string_view GetDescription() {
    return "Text fixes in Gorou's Apartment.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6600.dat",
            17521,
            HyoutaUtils::Hash::SHA1FromHexString("d056dcc6da6e8fd5c8685051a25f81aa5a8463f4"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "No change from Awakening Phase."
        // actual image and other instances of this text have 'for' instead of 'from'
        patcher.ReplacePartialCommand(0x1c69, 0x45, 0x1c98, 0x3, {{0x6f, 0x72}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6600

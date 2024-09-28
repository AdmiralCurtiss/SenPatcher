#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::tk_hekiru {
std::string_view GetDescription() {
    return "Text fixes in conversations with Hekiru.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_hekiru.dat",
            18081,
            HyoutaUtils::Hash::SHA1FromHexString("101c9596244d04e16be2bdaaa1f67b21dba7d44e"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "journalismclub" -> "journalism club"
        patcher.ExtendPartialCommand(0x3915, 0x99, 0x39a3, {{0x20}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_hekiru

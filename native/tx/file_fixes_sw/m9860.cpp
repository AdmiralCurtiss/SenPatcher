#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

namespace SenLib::TX::FileFixesSw::m9860 {
std::string_view GetDescription() {
    return "Text fixes in Sky Tomb Boundary (last room).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m9860.dat",
            49137,
            HyoutaUtils::Hash::SHA1FromHexString("d717d560308cc6bd3067a7acc64a58d352e8f0d4"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2PYes, it would be impossible#10W...\x01#1000WUnder normal circumstances, that is."
        // patcher.ReplacePartialCommand(0x63bc, 0x5a, 0x63ea, 0x2a, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m9860

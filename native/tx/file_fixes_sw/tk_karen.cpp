#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_ztk_karen[] = "Text fixes in conversations with Karen.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_karen {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_karen.dat",
            17329,
            HyoutaUtils::Hash::SHA1FromHexString("ba5442534dd59242da8179bdc722d30768b7a1e8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "It was such an important thing\x01that Daddy gave me, too…"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x2ca7, 0x6e, 0x2caa, 0x69, "");
        bin[0x2D10] = '.';
        bin[0x2D11] = '.';
        bin[0x2D12] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_karen

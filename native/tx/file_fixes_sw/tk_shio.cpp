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
__declspec(dllexport) char SenPatcherFix_1_ztk_shio[] = "Text fixes in conversations with Shio.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_shio {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_shio.dat",
            11129,
            HyoutaUtils::Hash::SHA1FromHexString("4eed1d02db66eb6afcb892b083427f084488f4f8"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K(…Those two have nothing to\x01worry about.)"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x1feb, 0x32, 0x1fee, 0x2d, "");
        bin[0x1FF1] = '.';
        bin[0x1FF2] = '.';
        bin[0x1FF3] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_shio

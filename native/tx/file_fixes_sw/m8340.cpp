#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "tx/file_fixes_sw/dungeon_names.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_m8340[] = "Text fixes in " DUNGEON_NAME_m8340 ".";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m8340 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8340.dat",
            18465,
            HyoutaUtils::Hash::SHA1FromHexString("d4a419f6cb943d90682eb6ee0e7a50d3931c4386"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#2PYou bet it you're gonna get my\x01A-game!"
        // (see also other parts of this dungeon)
        // -> "bet! You're"
        patcher.ReplacePartialCommand(0x392c, 0x33, 0x393E, 5, STR_SPAN("! Y"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8340

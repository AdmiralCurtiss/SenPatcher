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
__declspec(dllexport) char SenPatcherFix_1_m8330[] = "Text fixes in " DUNGEON_NAME_m8330 ".";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m8330 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8330.dat",
            19617,
            HyoutaUtils::Hash::SHA1FromHexString("0ee11754b692385582707f33358e163ed55cb061"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1PYou bet it you're gonna get my\x01A-game!"
        // (see also other parts of this dungeon)
        // -> "bet! You're"
        patcher.ReplacePartialCommand(0x3f3e, 0x33, 0x3F50, 5, STR_SPAN("! Y"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8330

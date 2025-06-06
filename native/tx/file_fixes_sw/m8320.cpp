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
__declspec(dllexport) char SenPatcherFix_1_m8320[] = "Text fixes in " DUNGEON_NAME_m8320 ".";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m8320 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8320.dat",
            15809,
            HyoutaUtils::Hash::SHA1FromHexString("270e7f260bb9c15470d8b57540c2f0eed7144c17"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#1PYou bet it you're gonna get my\x01A-game!"
        // (see also other parts of this dungeon)
        // -> "bet! You're"
        patcher.ReplacePartialCommand(0x327c, 0x33, 0x328E, 5, STR_SPAN("! Y"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8320

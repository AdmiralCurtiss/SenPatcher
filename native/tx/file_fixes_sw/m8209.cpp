#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
// reunite with Rion/Mitsuki
__declspec(dllexport) char SenPatcherFix_1_m8209[] = "Text fixes in Lethe Pillar (midpoint).";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::m8209 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/m8209.dat",
            19225,
            HyoutaUtils::Hash::SHA1FromHexString("76af667ab173159138a50826f2da5238e12e85a7"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#4S#6P#800WYou#50W...\x01#1000W#5SYou idiot!"
        // remove the '#50W'
        // (EV_08_12_01)
        patcher.RemovePartialCommand(0x2f22, 0x33, 0x2F38, 4);

        // "#2K#0TWe do. My grandfather procured a building\x01within city limits as a precautionary
        // measure for Hollow Quake."
        // needs an extra linebreak and some kind of article for Hollow Quake
        // (EV_08_12_01)
        bin[0x393B] = 0x1;
        patcher.ExtendPartialCommand(0x38de, 0xc0, 0x3948, STR_SPAN("a "));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::m8209

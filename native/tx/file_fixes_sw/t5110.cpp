#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t5110[] = "Text fixes in Nanahoshi Mall 1F.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t5110 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t5110.dat",
            224609,
            HyoutaUtils::Hash::SHA1FromHexString("15f7ad272c2ff3ed8c8a89140753296f8ae7f2c9"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Heh heh. They always do a final\x01check of everyone's equipment\x01the final battle."
        // This is missing a word, 'before' probably. The line in Switch 1.0.0 was:
        // "Heh heh. They always do a final\x01check of everyone's equipment\x01before airsoft
        // tournament finals."
        // so they probably just removed a word too much when rephrasing
        // (NPC dialogue with Mashima during the last free roam before the final dungeon in the
        // Final Chapter)
        patcher.ExtendPartialCommand(0xdf74, 0xe8, 0xe010, STR_SPAN("before "));

        // "#1PDamn. Never expected Something like\x01this to spawn an Otherworld gate."
        // decapitalize 'Something'
        // (Chapter 6, incident with the Alisa clock, Shio must be in party (EV_06_03_01))
        // patcher.ReplacePartialCommand(0x184ef, 0x4d, 0x184f2, 0x48, "");
        bin[0x1850A] = 's';

        // "Let's see. A cute notebook, a pink\x01clock… Snagged some cheap nail\x01stickers,
        // too..."
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x1060e, 0xbf, 0x10611, 0x54, "");
        bin[0x10639] = '.';
        bin[0x1063a] = '.';
        bin[0x1063b] = '.';

        // the "Obtained Biting Wolf Set x2" message is missing a space before the "x2"
        patcher.ExtendPartialCommand(0x2c843, 0x1e, 0x2c858, {{' '}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t5110

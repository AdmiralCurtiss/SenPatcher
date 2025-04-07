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
__declspec(dllexport) char SenPatcherFix_1_s2000[] = "Text fixes in Memorial Park.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s2000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s2000.dat",
            513649,
            HyoutaUtils::Hash::SHA1FromHexString("fd108b3d4e84603b6504b7b7a44375c430c7e6b7"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // Expert -> Advanced
        // to match what the skateboard levels are actually called in the minigame
        patcher.ReplacePartialCommand(0x721c6, 0x23, 0x721e1, 6, STR_SPAN("Advanced"));
        patcher.ReplacePartialCommand(0x7223c, 0x23, 0x72257, 6, STR_SPAN("Advanced"));

        // TODO
        // "#K#0TLooks like everyone's here."
        // Nonsense in context. This is during the concert quest in the epilogue.
        // patcher.ReplacePartialCommand(0x567b8, 0x25, 0x567bd, 0x1e, "");

        // "#2P#F#5S...is going to be be..."
        // one 'be' too much (concert quest in the epilogue)
        patcher.RemovePartialCommand(0x5c140, 0x2a, 0x5C15F, 3);

        // "#K(This guy is still at it... He's gonna\x01be cold if he keeps aat it till nighttime.)"
        // 'aat' -> 'at'
        // (NPC dialogue with Takeuchi at the start of the After Story)
        patcher.RemovePartialCommand(0x19416, 0x59, 0x19457, 1);

        // "Apparently there's was some kind\x01of unquenchable fire burning in\x01Houraichou."
        // 'there's' -> 'there'
        patcher.RemovePartialCommand(0x19f19, 0xe4, 0x19F2C, 2);

        // "It's awesome that people from Morimiya\x01were actually gonna be in the concert."
        // very strange in context, drop the 'gonna be' to make it make sense
        // (after the concert sidequest in the epilogue)
        patcher.RemovePartialCommand(0x1d089, 0x7f, 0x1D0C1, 9);

        // "The weather's so nice today!\x01I'm taking advantage of it\x01by eat out."
        // 'eat out' -> 'eating out'
        // (old man at the tables in the epilogue)
        patcher.ExtendPartialCommand(0x24e50, 0x77, 0x24EC0, STR_SPAN("ing"));

        // "We had a long weekend Immediately \x01after the incident."
        // decapitalize 'Immediately'
        // (EV_11_00_01)
        // patcher.ReplacePartialCommand(0x4838b, 0x41, 0x4838e, 0x3c, "");
        bin[0x483AA] = 'i';

        // "…I decided to do some personal\x01 training with Chiaki."
        // unicode ellipsis and extra space
        // (EV_11_00_01)
        bin[0x48441] = '.';
        bin[0x48442] = '.';
        bin[0x48443] = '.';
        patcher.RemovePartialCommand(0x48438, 0x42, 0x48462, 1);

        // "#2P#0T*gulp*#20W #1000W*gulp*#25W...#1000W"
        // works surprisingly okay, but I'm still gonna remove the '#25W' here
        // (EV_11_00_01)
        patcher.RemovePartialCommand(0x49258, 0x30, 0x49278, 4);

        // "#K(Wh-Whoa. He's just a part-timer...\x01right?"
        // missing closing parenthesis
        // (NPC dialogue with Yuusuke in the After Story)
        patcher.ExtendPartialCommand(0x15cb1, 0x31, 0x15CE0, {{')'}});

        // "#E[G]Everyone played so well.\x01Even I felt like I was performing a\x01better than
        // usual."
        // "a better" -> "better"
        // (NPC dialogue with Wakaba after the concert in the Epilogue)
        patcher.RemovePartialCommand(0x26c3e, 0xe2, 0x26CA7, 2);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2000

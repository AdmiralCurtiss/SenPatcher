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

        // "It turned out to be a piece of glass. What a pane in the…"
        // ellipsis -> three dots
        // (QS_0504_02_D, that's the sidequest where you try to find something the kid lost)
        // patcher.ReplacePartialCommand(0x4e96f, 0x41, 0x4e972, 0x3c, "");
        bin[0x4E9AB] = '.';
        bin[0x4E9AC] = '.';
        bin[0x4E9AD] = '.';

        // There's a super confusing phrasing in the Epilogue quest where you throw the concert in
        // the park. After you've asked the three guests whether they want to be the backing band
        // and come back here, you get this conversation:
        //
        // City Hall Employee: "#K#0THey, guys! I've been waiting for you!"
        // City Hall Employee: "#E[9]#M_0SPiKA's manager called me earlier."
        // City Hall Employee: "#E[5]So it's going to be SPiKA with a
        //                      backing band of Morimiya locals.
        //                      This is going to be awesome!"
        // Rion: "#K#0THeh heh.\x01Right?!"
        // Kou: "#K#0TLooks like everyone's here."
        // Mitsuki: "#K#0TRight after we decided on the details,\x01I asked everyone to meet here."
        // City Hall Employee: "#K#0TOh. Thank you!\x01I mean it─thank you so, so much!"
        // [...]
        // Asuka: "#K#0TConsidering how long it takes to
        //         set up and rehearse, the sooner we
        //         meet up, the better."
        // City Hall Employee: "#K#0TThe stage is all good to go.
        //                      We're ready when you are."
        // City Hall Employee: "#E_0#M_0Do you want some time to
        //                      warm up?"
        // > Get Ready
        //   Hold Off
        // Kou: "#K#0TYeah, we're good.\x01We'll reach out\x01to everyone now."
        // Rion: "#K#0TYup! I'll let the rest of SPiKA know."
        //
        // I genuinely have no idea what's going on here. This conversation is trapped in a weird
        // superposition where everyone is already here and also still needs to be contacted to come
        // here. My *best guess* on what's going on here is that Kou's "Looks like everyone's here."
        // actually means "Looks like SPiKA's all here already.", that way the rest kinda sorta
        // makes sense I guess, even though visually nothing indicates that Spika is here? So let's
        // change that line for lack of better ideas...
        patcher.ReplacePartialCommand(0x567b8,
                                      0x25,
                                      0x567c0,
                                      0x1b,
                                      STR_SPAN("Looks like all of SPiKA\x01"
                                               "is already here."));


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s2000

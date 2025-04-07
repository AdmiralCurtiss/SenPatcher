#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s4100[] = "Text fixes in Kou's Room.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s4100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s4100.dat",
            161737,
            HyoutaUtils::Hash::SHA1FromHexString("f692548a9841546b9ee4b3f7b99e1c2d2594075f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "##0T#5SAAAHH!"
        // double '#' in front
        // (EV_07_00_00)
        patcher.RemovePartialCommand(0x551a, 0x18, 0x5523, 1);

        // sync memories menu to actual game for the hangout events:

        // "#3KThat's not what I was worrying\x01about, but all right..."
        // '...' -> '.'
        patcher.RemovePartialCommand(0x186d8, 0x3e, 0x18711, 2);

        // "#E[9]#M_0Thought it'd be a total pigsty\x01without your mom tellin' you to\x01clean it
        // up."
        // newlines
        // patcher.ReplacePartialCommand(0x18bea, 0x8f, 0x18c28, 0x4f, "");
        std::swap(bin[0x18C67], bin[0x18C6A]);

        // "#E[N]#M_0Prepare yourself, Kou, 'cause I've been\x01waiting for this! Today's the day I
        // uncover\x01your deeply buried perversions!"
        // "waiting" -> "waitin'"
        // patcher.ReplacePartialCommand(0x19743, 0x12a, 0x1978e, 0x77, "");
        bin[0x197C0] = '\'';

        // "#K#0TGod, what are you, 12?!\x01Stop looking through my stuff!"
        // '12?!' -> 'twelve?! '
        patcher.ReplacePartialCommand(0x19923, 0x40, 0x1993E, 4, STR_SPAN("twelve?! "));

        // "#E[1]#M_9I shouldn't be surprised it's\x01so good, since your grandma\x01made it."
        // linebreak, 'grandma' -> 'mom'
        std::swap(bin[0x1AAC2], bin[0x1AAC5]);
        patcher.ReplacePartialCommand(0x1aa3d, 0xac, 0x1AAD7, 8, STR_SPAN("mom "));

        // "#K#F#0TOh, this doll is so cute!\x01It's Morimaru, right?"
        // 'this' -> 'that'
        // patcher.ReplacePartialCommand(0x1ae9c, 0x3b, 0x1aea3, 0x32, "");
        bin[0x1AEAC] = 'a';
        bin[0x1AEAD] = 't';

        // "#E[5]#M_0I'll go grab the album so I can show\x01you there's more to stargazing\x01than
        // just looking up at the sky!"
        // linebreaks
        // patcher.ReplacePartialCommand(0x1b71d, 0xac, 0x1b75f, 0x68, "");
        std::swap(bin[0x1B7A6], bin[0x1B7AB]);

        // "#E_8#M_9But I gotta admit, games have come\x01pretty far. It's like playing a\x01movie."
        // linebreaks
        // patcher.ReplacePartialCommand(0x1d605, 0x7f, 0x1d635, 0x4d, "");
        bin[0x1D67B] = ' ';

        // "After a slow start, Shio's natural fighting skills and instincts\x01kicked in, allowing
        // him to school Yuuki instead."
        // linebreaks
        // patcher.ReplacePartialCommand(0x1e297, 0x77, 0x1e29a, 0x72, "");
        bin[0x1E2CD] = 0x1;
        bin[0x1E2DB] = ' ';
        bin[0x1E2FD] = 0x1;

        // "In the end, Shio won by a landslide, making Yuuki quite angry\x01at losing at his own
        // game. Kou had to diffuse the situation\x01before the fight made it to the real world."
        // linebreaks
        // patcher.ReplacePartialCommand(0x1e4a6, 0xab, 0x1e4a9, 0xa6, "");
        std::swap(bin[0x1E4E1], bin[0x1E4E7]);
        std::swap(bin[0x1E519], bin[0x1E523]);

        // "#1K#FReally? Maybe I should give it\x01a try myself."
        // second sentence is completely different
        patcher.ReplacePartialCommand(0x20b7a, 0x36, 0x20B8A, 0x24, STR_SPAN("Perhaps I should."));

        // "#1POh, you weren't kidding. It IS\x01comfortable. The mattress really\x01makes the whole
        // world of difference."
        // "the whole" -> "a whole"
        patcher.ReplacePartialCommand(0x20d4f, 0xbb, 0x20D9B, 3, STR_SPAN("a"));

        // "#E[99999999A]#M_0Ahh. It's such a high-quality\x01figure. You can totally even\x01check
        // out under the skirt─"
        // second sentence is completely different
        patcher.ReplacePartialCommand(0x22ae1,
                                      0xb9,
                                      0x22B67,
                                      0x31,
                                      STR_SPAN("Now, what did they\x01"
                                               "put under the skirt?"));

        // "#K#F#0TKnock it off!\x01(I'm already regretting\x01this purchase.)"
        // linebreaks
        // patcher.ReplacePartialCommand(0x22bb3, 0x41, 0x22bba, 0x38, "");
        std::swap(bin[0x22BE2], bin[0x22BE7]);

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s4100

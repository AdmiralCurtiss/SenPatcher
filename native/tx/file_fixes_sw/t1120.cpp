#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1120[] = "Text fixes in school 2F.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1120 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1120.dat",
            405409,
            HyoutaUtils::Hash::SHA1FromHexString("40e32214fecb0d37e97cc57366b0a54cc20fe414"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // linebreaks
        // "#E_0#M_0There's also a rumor that the grand\x01finale will be on Sunshine Road."
        // (EV_17_01_02)
        std::swap(bin[0x4a271], bin[0x4a278]);
        // "#2PI always hear her complaining\x01about something or other when\x01we finish Student
        // Council meetings."
        // (EV_17_01_04)
        std::swap(bin[0x4c185], bin[0x4c188]);


        // Yuuki final bond event
        // "#800W*click* *click* *click*\x01#9W*sigh* No luck."
        // remove that "#9W" tag, otherwise the box taxes forever to print
        patcher.RemovePartialCommand(0x4e730, 0x39, 0x4e755, 3);

        // "#2P#800WWork, spending time to friends, going\x01to class… Maybe even getting my
        // ass\x01handed to me in games."
        // … -> ... and "time to friends" -> "time with friends"
        bin[0x4F2B7] = '.';
        bin[0x4F2B8] = '.';
        bin[0x4F2B9] = '.';
        patcher.ReplacePartialCommand(0x4f279, 0x74, 0x4F29D, 2, STR_SPAN("with"));

        // "#1PBut If I do something THAT off the wall, \x01
        // there's no way we'll trip up any death\x01
        // flags. No one would see that coming."
        // 'If' -> 'if';
        bin[0x4F5F2] = 'i';
        bin[0x562DE] = 'i'; // memories menu also


        // memories menu inconsistencies for Yuuki
        // "#800W*click* *click* *click*\x01#9W*sigh* #1000WNo luck."
        // "#9W*sigh* #1000W" -> "*sigh* "
        patcher.ReplacePartialCommand(0x55410, 0x3f, 0x55435, 0x10, STR_SPAN("*sigh* "));

        // "But I can't find a way to get\x01inside Pandora. There might\x01be none."
        // "be none" -> "not be one"
        patcher.ReplacePartialCommand(0x555c6, 0x4c, 0x55608, 7, STR_SPAN("not be one"));

        // "Yeah, yeah. I was planning\x01on stopping soon anyway."
        // "soon anyway" -> "soon, anyway"
        patcher.ExtendPartialCommand(0x55681, 0x3d, 0x556B4, STR_SPAN(","));

        // "#2P#800WWork, spending time to friends, going\x01to class. #12W...#1000WMaybe even
        // getting my ass\x01handed to me in games."
        // ". #12W...#1000W" -> "... " and "time to friends" -> "time with friends"
        {
            std::vector<char> tmp;
            tmp.assign(&bin[0x55f5b], &bin[0x55f5b] + 0x7f);
            constexpr auto dots = STR_SPAN("... ");
            tmp.erase(tmp.begin() + 0x3e, tmp.begin() + 0x3e + 0xf);
            tmp.insert(tmp.begin() + 0x3e, dots.begin(), dots.end());
            constexpr auto with = STR_SPAN("with");
            tmp.erase(tmp.begin() + 0x24, tmp.begin() + 0x24 + 2);
            tmp.insert(tmp.begin() + 0x24, with.begin(), with.end());
            patcher.ReplaceCommand(0x55f5b, 0x7f, tmp);
        }

        // "That sounds capital L ＂Lame.＂"
        // "capital L ＂Lame.＂" -> "capital-L ＂lame.＂"
        // patcher.ReplacePartialCommand(0x56004, 0x6b, 0x56007, 0x26, "");
        bin[0x5601F] = '-';
        bin[0x56025] = 'l';

        // "#2PGee, thanks for the vote of\x01confidence."
        // "Gee" -> "Geez"
        patcher.ExtendPartialCommand(0x56097, 0x8d, 0x560A5, STR_SPAN("z"));

        // "#E[1]#M_9Don't pull an all-nighter on\x01videogames, though."
        // "videogames" -> "video games"
        patcher.ExtendPartialCommand(0x56a10, 0x56, 0x56A56, STR_SPAN(" "));


        // memories menu inconsistencies for Shiori
        // "#E[9]#M[0](It didn't even change after this\x01whole mess with the Eclipse\x01started.)"
        // "Eclipse" -> "Otherworld"
        patcher.ReplacePartialCommand(0x5714d, 0x8d, 0x571C7, 7, STR_SPAN("Otherworld"));

        // "#3C#3CWhether it was at home or at school,\x01you were always there for me. If I
        // needed\x01you, I knew you'd be there."
        // linebreak location difference
        // patcher.ReplacePartialCommand(0x57da9, 0x7b, 0x57db5, 0x6d, "");
        std::swap(bin[0x57DFD], bin[0x57E06]);

        // "#3C#3CBut, even if it did..."
        // remove comma
        patcher.RemovePartialCommand(0x580c1, 0x60, 0x580D3, 1);

        // "#3C#3CThere are a lot of unknowns out there,\x01but I know that's a constant. I know it
        // in\x01my heart of hearts."
        // linebreak location difference
        // patcher.ReplacePartialCommand(0x5826e, 0x76, 0x5827a, 0x68, "");
        std::swap(bin[0x582C8], bin[0x582CE]);

        // "#3C#3CNo matter what may come, this\x01much will always be true. I know it\x01deep in my
        // soul."
        // linebreak location difference
        // patcher.ReplacePartialCommand(0x58369, 0xe2, 0x583f4, 0x55, "");
        std::swap(bin[0x58414], bin[0x58419]);
        std::swap(bin[0x58438], bin[0x58440]);

        // "#3K(You saved me in so many\x01ways, and you don't even\x01know it.)"
        // "You" -> "You've"
        patcher.ExtendPartialCommand(0x58836, 0x9c, 0x58845, STR_SPAN("'ve"));


        // "#E_8#M_4#4S#600WKou, Chizuru... If I die today,\x01I'll die a happy man."
        // harmless
        // patcher.ReplacePartialCommand(0x6ab4, 0x5e, 0x6ad6, 0x3a, "");

        // "#1P#800W(She totally did that on purpose!)\x01#1000W#5SWhat a pain in the..."
        // harmless
        // (EV_06_06_01)
        // patcher.ReplacePartialCommand(0x35979, 0x53, 0x359b2, 0x18, "");

        // "#2P#5S#5S#500WShut it."
        // harmless
        // (EV_17_01_04)
        // patcher.ReplacePartialCommand(0x4bc38, 0x20, 0x4bc49, 0xd, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1120

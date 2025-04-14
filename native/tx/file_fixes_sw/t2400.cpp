#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t2400[] = "Text fixes in Sunshine Road.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t2400 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t2400.dat",
            373257,
            HyoutaUtils::Hash::SHA1FromHexString("fdd090578225d6b9a4e28b4b7a06b36a83d34a5f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#5S#0THaruna"
        // memories menu of the After Story concert has this instead of the correct line, no idea
        // what happened here...
        patcher.ReplacePartialCommand(
            0x4d6e9, 0x11, 0x4d6f2, 0x6, STR_SPAN("The 2015 Morimiya Autumn Festival!"));

        // "#K##0T＂Girl talk＂?\x01
        // Come on... Not cool, you guys."
        // double '#' in front
        // (Conversation with Shiori/Ryouta/Chizuru/Jun in the final Sunshine Road event)
        patcher.RemovePartialCommand(0xe9d6, 0x3a, 0xe9dc, 1);

        // "#3C#500W#0T#3C...But...I just...#14W#250Wdon't...understand..."
        // "#3C#500W#1P#3CTen years ago...#16W Wh-#250WWhy...#22W #250WWhy#16W...#250Wwas I
        // born...?"
        // drop all the text delay commands except the first one, otherwise these flashbacks are a
        // pain in the ass to sit through
        // see also m9870 where the original non-flashback lines are
        // (two copies in EV_17_27_03 and EV_17_31_00)
        for (int offset : {0x290a6, 0x3782c}) {
            static constexpr size_t size = 0xa7;

            std::vector<char> tmp;
            tmp.reserve(size);
            tmp.insert(tmp.end(), &bin[offset], &bin[offset + 0x28]);
            tmp.insert(tmp.end(), &bin[offset + 0x31], &bin[offset + 0x6a]);
            bool drop = false;
            for (size_t i = 0x6a; i < size; ++i) {
                const char c = bin[offset + i];
                if (c == '#') {
                    drop = true;
                }
                if (!drop) {
                    tmp.push_back(c);
                }
                if (drop && c == 'W') {
                    drop = false;
                }
            }
            patcher.ReplaceCommand(offset, size, tmp);
        }

        // "#K#0T#FGod damn it...\x01#1000W#5SGot it! I'll be right there!"
        // harmless
        // (EV_17_32_00)
        // patcher.ReplacePartialCommand(0x41835, 0x45, 0x41859, 0x1f, "");

        // "#5S#1PAll right, Morimiya!\x01#1000WLet's make this a night to remember!"
        // harmless
        // (EV_17_33_00)
        // patcher.ReplacePartialCommand(0x446ae, 0x50, 0x446d1, 0x2a, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t2400

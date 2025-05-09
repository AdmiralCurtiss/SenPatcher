#include <cstring>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t5120[] = "Text fixes in Nanahoshi Mall 2F.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t5120 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t5120.dat",
            159017,
            HyoutaUtils::Hash::SHA1FromHexString("974dd2e756523414e07e058319ac3f6206d55cbb"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);
        using HyoutaUtils::Vector::ShiftData;

        // "#K#0TNo clue what that is"
        // missing period
        // (EV_06_03_00, Shio in party)
        patcher.ExtendPartialCommand(0x13a12, 0x1e, 0x13A2E, {{'.'}});

        // the magical alisa gacha list of figures needs a few fixes
        {
            static constexpr size_t offsetSeries1 = 0x252cc;
            static constexpr size_t offsetSeries2 = 0x251a3;
            static constexpr size_t sizeSeries1 = 0xa1;
            static constexpr size_t sizeSeries2 = 0x109;
            std::vector<char> onlySeries1;
            onlySeries1.resize(sizeSeries1);
            std::vector<char> withSeries2;
            withSeries2.resize(sizeSeries2);
            std::memcpy(onlySeries1.data(), &bin[offsetSeries1], sizeSeries1);
            std::memcpy(withSeries2.data(), &bin[offsetSeries2], sizeSeries2);

            // make the headers consistent with itself and the Kagemaru set
            onlySeries1[0x1f] = 0x01;
            withSeries2[0x1f] = 0x01;
            ShiftData(onlySeries1, 0x17, 0x29, 9);
            ShiftData(withSeries2, 0x17, 0x29, 9);
            onlySeries1[0x1d] = 0x3a;
            withSeries2[0x1d] = 0x3a;
            onlySeries1.insert(onlySeries1.begin() + 0x28, (char)0x3a);
            withSeries2.insert(withSeries2.begin() + 0x28, (char)0x3a);
            withSeries2.erase(withSeries2.begin() + 0xa9, withSeries2.begin() + 0xaf);
            withSeries2[0xa9] = 0x3a;

            // list misspells Sara as Sarah
            onlySeries1.erase(onlySeries1.begin() + 0x66);
            withSeries2.erase(withSeries2.begin() + 0x66);


            // apply
            patcher.ReplaceCommand(offsetSeries1, sizeSeries1, onlySeries1);
            patcher.ReplaceCommand(offsetSeries2, sizeSeries2, withSeries2);
        }

        // "Five base figures and a secret one..."
        // wrong, it's five in total, one of which is secret. just change to four + secret...
        bin[0x2413F] = 'o';
        bin[0x24140] = 'u';
        bin[0x24141] = 'r';

        // "There's five base figures and a secret one."
        // same thing here
        bin[0x240D9] = 'o';
        bin[0x240DA] = 'u';
        bin[0x240DB] = 'r';

        // "#500W#3C#1P#3CHow frustrating...#20W\x01#300WHow sad...#15W\x01#600W...How fun."
        // harmless
        // (EV_17_16_01)
        // patcher.ReplacePartialCommand(0x160fa, 0x53, 0x1613b, 0x10, "");

        // "Mechanical Voice" -> "Robotic Voice" for the Horus line in EV_17_16_01 for consistency
        patcher.ReplacePartialCommand(0x17938, 0x14, 0x17939, 0xa, STR_SPAN("Robotic"));

        // "Heehee. She's a customer who's\x01been coming and really stands out."
        // This line is just nonsense. She's referring to Airi (who's standing in the shop but not
        // directly talking to her) and as far as I can tell this is supposed to be something like
        // 'this girl over there has been shopping here every once in a while and she really stands
        // out', but as written the sentence just makes no sense.
        // (Chapter 4, at night before leaving for the abandoned factory)
        patcher.ReplacePartialCommand(0x9753,
                                      0xf6,
                                      0x9756,
                                      0x41,
                                      STR_SPAN("That girl over there really\x01"
                                               "stands out, doesn't she?"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t5120

#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s8000[] = "Text fixes in NDF base.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::s8000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s8000.dat",
            187041,
            HyoutaUtils::Hash::SHA1FromHexString("9e012381408e773404bfef6a0c2235483b7a85a1"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "But you're a kid." -> "But you're kids!"
        // the entire XRC is standing there and is saying 'we're gonna do this!!',
        // it's very weird for him to only address one
        // (EV_07_27_02)
        patcher.ReplacePartialCommand(0x8b8a, 0x1e, 0x8BA0, 6, STR_SPAN("kids!"));

        // "#0THey…Saka..."
        // ellipsis to dots, and also decapitalize the Saka because it's the second part of Tokisaka
        // (EV_07_32_03)
        // patcher.ReplacePartialCommand(0xdab5, 0x1b, 0xdabd, 0x10, "");
        bin[0xDAC3] = '.';
        bin[0xDAC4] = '.';
        bin[0xDAC5] = '.';
        bin[0xDAC6] = 's';

        // "#2P#7C#800W#7CIt's about the traces of twilight.#18W\x01#1000WThey're still here."
        // harmless
        // (EV_07_32_03)
        // patcher.ReplacePartialCommand(0x1051a, 0x58, 0x10557, 0x19, "");

        // "#E_F#M_A#2P#800WAre you... #10W #800WAre you at Acros Tower?"
        // delay is okay, but remove the second space
        // (EV_07_32_03)
        patcher.RemovePartialCommand(0x11099, 0x46, 0x110BB, 1);

        // "#2C#0T#200W#2CHurry. #500WGet out of the city...!"
        // harmless
        // (EV_07_32_03)
        // patcher.ReplacePartialCommand(0x1115f, 0x3c, 0x1117c, 0x1c, "");

        // "#2P#5S#1C#0T#800W#1CBlade of perpetual ice!\x01#23W#1000WIcicle Nova!"
        // harmless
        // (EV_17_21_02)
        // patcher.ReplacePartialCommand(0x1c1a4, 0x4d, 0x1c1dc, 0x12, "");

        // "#E[7]#M_A#2K#FI can't fight anymore.\x01#1000W#5SRun, Shiori!"
        // harmless
        // (EV_17_21_02)
        // patcher.ReplacePartialCommand(0x1e47f, 0x44, 0x1e4b2, 0xf, "");

        // "#2P#3C#600W#3CHow about you come play\x01with me instead, Shiori?"
        // harmless
        // (EV_17_21_02)
        // patcher.ReplacePartialCommand(0x21653, 0x48, 0x21666, 0x33, "");

        // "#0T#1C#1CSearch complete#16W...#1000W\x01No results found."
        // for consistency use an emdash instead of three dots here, this also fixes the delay
        // (EV_17_22_02)
        // patcher.ReplacePartialCommand(0x253e8, 0x41, 0x2540f, 0x18, "");
        bin[0x2540C] = '\xE2';
        bin[0x2540D] = '\x94';
        bin[0x2540E] = '\x80';

        // "Mechanical Voice" -> "Robotic Voice" for the above line for consistency
        patcher.ReplacePartialCommand(0x253c5, 0x14, 0x253c6, 0xa, STR_SPAN("Robotic"));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s8000

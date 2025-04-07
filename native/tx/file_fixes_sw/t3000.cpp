#include <cstring>
#include <span>
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
__declspec(dllexport) char SenPatcherFix_1_t3000[] = "Text fixes in Shopping Street.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t3000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3000.dat",
            376153,
            HyoutaUtils::Hash::SHA1FromHexString("dbbb29291256c62a63e86c73bf7e34b88c313109"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // the Kagemaru Series 2 list has a few minor issues
        // Switch v1.0.1 has improved this but it's still not perfect
        {
            static constexpr size_t offset = 0x56d29;
            static constexpr size_t size = 0xd3;
            std::vector<char> tmp;
            tmp.resize(size);
            std::memcpy(tmp.data(), &bin[offset], size);

            // Love-struck -> Love-Struck to match item name
            tmp[0x9D] = 'S';

            // swap Love-Struck and Pudgy to match item order in menu
            HyoutaUtils::Vector::ShiftData(tmp, 0x98, 0xbc, 0x15);

            // extra line break between Series 1 and Series 2
            tmp.insert(tmp.begin() + 0x7b, (char)0x01);

            // apply
            patcher.ReplaceCommand(offset, size, tmp);
        }

        // "Monsters have been rampaging\x01through Shopping Street.."
        // double period
        // (NPC dialogue with woman near Sora's apartment, Final Chapter first possible opportunity
        // in Shopping District)
        patcher.RemovePartialCommand(0x20ef8, 0x80, 0x20f30, 0x1);

        // "It seems like everyone from the\x01businesses on Shopping Street is\x01collaborated on
        // Morimiya's entry."
        // 'is' -> 'has'
        // (NPC Seiko in Epilogue)
        patcher.ReplacePartialCommand(0x168fa, 0x122, 0x169AF, 2, STR_SPAN("has"));

        // "#E_E#M_0It must have been tough going\x01through that labyrinth from\x01the very first
        // day, right?"
        // Switch v1.0.1 changed this from 'those labyrinths' to 'that labyrinth' and I'm not sure
        // why because it makes less sense now? Revert that.
        // (EV_06_05_02)
        patcher.ReplacePartialCommand(0x2b205, 0x8f, 0x2B264, 0xe, STR_SPAN("those labyrinths"));

        // "#K#F#0T#4S#800W...#15W#1000W\x01#6SObviously!"
        // harmless
        // (EV_04_10_01)
        // patcher.ReplacePartialCommand(0x298e0, 0x34, 0x29905, 0xd, "");

        // "#1P#5SOh, wow...#13W#1000W\x01The weather's so nice today!"
        // harmless
        // (EV_11_00_00)
        // patcher.ReplacePartialCommand(0x3648f, 0x41, 0x364ab, 0x23, "");

        // "(Still, there was something about\x01that labyrinth... #8W#1000WOr maybe I'm
        // just\x01overthinking things.)"
        // harmless
        // (EV_11_03_02)
        // patcher.ReplacePartialCommand(0x39550, 0x6e, 0x3958f, 0x2d, "");

        // "#500W#3C#1P#3CHow frustrating...#20W\x01#300WHow sad...#15W\x01#600W...How fun."
        // harmless
        // (EV_17_16_00)
        // patcher.ReplacePartialCommand(0x3f802, 0x53, 0x3f843, 0x10, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3000

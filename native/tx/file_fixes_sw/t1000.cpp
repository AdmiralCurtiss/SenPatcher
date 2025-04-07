#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1000[] = "Text fixes outside school (including roof).";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1000.dat",
            335953,
            HyoutaUtils::Hash::SHA1FromHexString("c6b8de77bd347dd453ae02026170b080727d488f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#K#F#0TIt essentially destroyed the style\x01I'd nurtured to carefully during my\x01time
        // at Nemesis."
        // remove 'to' (asuka memories menu mismatch)
        patcher.RemovePartialCommand(0x40a4e, 0x68, 0x40A8D, 3);

        // "#7KYou wish. It was coincidence."
        // add 'a' (rion memories menu mismatch)
        patcher.ExtendPartialCommand(0x4378c, 0x92, 0x437A8, STR_SPAN("a "));

        // "Haha... Not even out of my depth.\x01I'm just freakin' lost."
        // add ', dude.' (ryouta memories menu mismatch)
        patcher.ReplacePartialCommand(0x45a5a,
                                      0x43,
                                      0x45A82,
                                      0x2,
                                      STR_SPAN(","
                                               "\x01"
                                               "dude. "));

        // "#2PRyouta."
        // add ellipsis (ryouta memories menu mismatch)
        patcher.ExtendPartialCommand(0x45aae, 0x45, 0x45AC0, STR_SPAN(".."));

        // "#0T#5S#800WAnnihilate!#20W\x01#7S#1000WAkashic Blade!"
        // scene EV_08_09_01
        // harmless
        // patcher.ReplacePartialCommand(0x1c942, 0x3d, 0x1c968, 0x14, "");

        // "#E[9999999999999999999999999999999998]#M_0#800WBut if I do...then tomorrow will
        // come.\x01If I stay up, I can delay it...even a little."
        // scene EV_08_20_01
        // harmless
        // patcher.ReplacePartialCommand(0x23a4f, 0x12c, 0x23ab8, 0x59, "");

        // "#E[999999999999999999999999999999FFFFFFFFF9]#M_A#600W...But here in Japan... And with a
        // boy...\x01#10W#3S#600WIt feels...different."
        // scene EV_08_20_01
        // harmless
        // patcher.ReplacePartialCommand(0x24c11, 0x8a, 0x24c7f, 0x1a, "");

        // "#0T#400WHaha...#10W\x01#6S#1200WHahahahaha!"
        // scene EV_08_20_01
        // harmless
        // patcher.ReplacePartialCommand(0x24d6c, 0x33, 0x24d8b, 0x11, "");

        // "#500WYeah. Let's, #7W#500WAsuka."
        // scene EV_08_20_01
        // harmless
        // patcher.ReplacePartialCommand(0x25550, 0x2a, 0x2556d, 0xb, "");

        // "#3K#F#1000WShoot, you're right!\x01#1000W#5SWe gotta book it!"
        // scene EV_13_04_00
        // harmless
        // patcher.ReplacePartialCommand(0x305f1, 0x44, 0x3061f, 0x14, "");

        // "#500WYeah, yeah. #1000WAnd wait a second!\x01#1000W#5SI told you not to call me that!"
        // scene EV_13_04_00
        // harmless
        // patcher.ReplacePartialCommand(0x306a3, 0x5c, 0x306db, 0x22, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1000

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
__declspec(dllexport) char SenPatcherFix_1_t6100[] = "Text fixes in Oasis Arcade.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t6100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6100.dat",
            92993,
            HyoutaUtils::Hash::SHA1FromHexString("1e4b391dc356208561ec5fda0015ca9c94c0f798"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // note: the color tags in the 'collect 1000 medals' system messages in the Tsubasa quest
        // look wrong but actually work fine, not sure how that works...

        // "#0T#2C#2CI have the raw materials, but...I won't \x01"
        // "be able to processes them that quickly."
        // "#2C#2CSo I'll need dried hellfire grass\x01"
        // "and ice crystal flowers─"
        // "#2C#2CTwo of each."
        // (this is the quest with the hot springs mom getting a fever, final chapter pre pandora)
        // This has tripped me up every time I've seen this scene. What he's supposed to be saying
        // here is "I have raw materials, but if I process them myself it's gonna take too long, so
        // I need you to find some pre-processed ones for me." but the way it's phrased always makes
        // me think 'wait if you already have them why do i have to bring them to you??'. Oh and the
        // "two of each" is just wrong as far as I can tell, it's just one of each. (though nitpick:
        // what the heck is the quantity of 'one grass')
        // I'll take some creative liberty and rewrite this to something more straightforward...
        patcher.ReplacePartialCommand(
            0x98c9,
            0xb6,
            0x98cf,
            0xae,
            STR_SPAN("#2CI have the raw materials, but I won't be able\x01"
                     "to process them quickly enough.\x02\x03"
                     "#2CSo I'll need you to find me some hellfire grass\x01"
                     "and an ice crystal flower, both already dried.\x02\x03"
                     "#2CCome by as soon as you have them."));

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6100

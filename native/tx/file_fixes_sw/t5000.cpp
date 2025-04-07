#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t5000[] = "Text fixes outside Nanahoshi Mall.";
}

namespace SenLib::TX::FileFixesSw::t5000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t5000.dat",
            42105,
            HyoutaUtils::Hash::SHA1FromHexString("826ed3c29a1f31652c5e5d6b0e4a14aa7e87ffb9"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "#800WThe Omega Drive has quite the\x01rebound."
        // This is somewhat bizarre... This is a leftover from the old English script that, for some
        // reason, decided to translate Asuka's limiters as a specific object. The Japanese script
        // just refers to it as 拘束術式解放, so I have no idea why they did this. I'm debating on
        // rewriting this line to just scrub the remaining mention of it, because every other
        // instance in the Switch script is just variations on "release binding spell" -- though
        // the costume item itself is still called Omega Drive even in the Switch version. So I'm
        // not sure what to do here. It's probably not particularly important anyway, to be honest.
        // (Chapter 3 Side Story, after the dungeon (EV_12_03_02))
        // patcher.ReplacePartialCommand(0x819b, 0x5e, 0x81a3, 0x2b, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t5000

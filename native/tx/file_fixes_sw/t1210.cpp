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
__declspec(dllexport) char SenPatcherFix_1_t1210[] = "Text fixes in school annex.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1210 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1210.dat",
            261569,
            HyoutaUtils::Hash::SHA1FromHexString("d39f0d5277f0bc7b89f8284e5949ddc29ced6c53"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // linebreaks
        // "#E_8#M_0But being able to hang out with\x01friends and join a club... I'm
        // actually\x01kind of jealous."
        std::swap(bin[0x3458e], bin[0x34597]);
        // "All encounters are once-in-a-lifetime\x01opportunities. We should cherish\x01them."
        bin[0x548a] = 0x20;

        // "#K#0THeard you guys were performing\x01at the festival, but I wasn't
        // exactly\x01expecting to find you in the clubroom."
        // "#K#0TEspecially not HERE of all places.\x01What're you doing in the clubroom?"
        // These are written as if they were part of the same conversation, but they're actually
        // mutually exclusive lines depending on if you've heard about Spika being at school before
        // or not. The first one is 'have heard' and the second one is 'have not heard'.
        // See also a similar pair of lines in t1310.
        patcher.ReplacePartialCommand(0x33b06,
                                      0x75,
                                      0x33b0e,
                                      0x6b,
                                      STR_SPAN("Heard you guys were practicing at\x01"
                                               "school, but I wasn't expecting\x01"
                                               "to run into you in our clubroom."));
        patcher.ReplacePartialCommand(0x33b92,
                                      0x4f,
                                      0x33b9a,
                                      0x45,
                                      STR_SPAN("What're you doing HERE in\x01"
                                               "our clubroom, of all places?"));

        // "#E[3333333333333333333336]#M_A#800WBut the being that caused this
        // one...\x01#14W#800WIt's currently hiding in Morimiya."
        // harmless
        // (EV_07_14_01)
        // patcher.ReplacePartialCommand(0x10dff, 0xdd, 0x10eb3, 0x27, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1210

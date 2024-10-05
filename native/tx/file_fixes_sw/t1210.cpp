#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1210 {
std::string_view GetDescription() {
    return "Text fixes in school annex.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1210.dat",
            261537,
            HyoutaUtils::Hash::SHA1FromHexString("43914132d11101d38ce55195442af3eb1ac12c41"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // linebreaks
        std::swap(bin[0x3457b], bin[0x34572]);
        bin[0x548e] = 0x20;

        // "Heard you guys were performing at the festival, but I wasn't exactly expecting to run
        // into you." "Especially not HERE of all places. What're you doing in the clubroom?"
        // These are written as if they were part of the same conversation, but they're actually
        // mutually exclusive lines depending on if you've heard about Spika being at school before
        // or not. The first one is 'have heard' and the second one is 'have not heard'.
        patcher.ReplacePartialCommand(0x33af6,
                                      0x69,
                                      0x33afe,
                                      0x5f,
                                      STR_SPAN("Heard you guys were practicing at\x01"
                                               "school, but I wasn't expecting\x01"
                                               "to run into you in our clubroom."));
        patcher.ReplacePartialCommand(0x33b76,
                                      0x4f,
                                      0x33b7e,
                                      0x45,
                                      STR_SPAN("What're you doing HERE in\x01"
                                               "our clubroom, of all places?"));

        // "#E[3333333333333333333336]#M_A#800WBut the being that caused this
        // one...\x01#14W#800WIt's currently hiding in Morimiya."
        // patcher.ReplacePartialCommand(0x10e03, 0xdd, 0x10eb7, 0x27, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1210

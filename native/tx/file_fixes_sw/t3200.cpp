#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3200[] = "Text fixes in Yanagi Sports.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t3200 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3200.dat",
            104225,
            HyoutaUtils::Hash::SHA1FromHexString("4127f845019f3f806e0deaa57c7379ecec101d33"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Kou, Shio, and Sora.."
        // add another period for ellipsis
        // In early Chapter 7, when investigating the 'slashing incident' at Yanagi's (EV_07_03_01).
        // Must have Sora in the party.
        patcher.ExtendPartialCommand(0x7eb5, 0x1a, 0x7ecb, {{0x2e}});

        // the "Obtained Jack-o'-Lantern x2" message is missing a space before the "x2"
        patcher.ExtendPartialCommand(0x15e58, 0x1e, 0x15e6d, {{' '}});

        // "#E[1]It looks like the store's interior\x01is back to normal now."
        // "I'm a bit disappointed in how\x01long the response took..."
        // Not sure what's up here. The store is still cut up.
        // (SB_08_10_00; come here during the pre-Pandora walkaround)
        // patcher.ReplacePartialCommand(0x17915, 0x93, 0x1792f, 0x3e, "");
        // patcher.ReplacePartialCommand(0x17915, 0x93, 0x1792f, 0x77, "");

        // "#2K#F...Or that's what I'd say, if we\x01hadn't already checked it out."
        // This follows "Okay, time to get back to business." so it doesn't make much sense. It's
        // supposed to be something like "We already expored Nanahoshi Mall though, so what are we
        // gonna do now?", it was just written under the assumption that the previous line already
        // mentioned the mall.
        // (EV_17_11_02, if you've already done the mall)
        // We'll just alter this slightly, "already checked it out" -> "already checked out
        // Nanahoshi Mall", plus move some linebreaks so that looks okay.
        std::swap(bin[0xE302], bin[0xE309]);
        std::swap(bin[0xE34C], bin[0xE354]);
        patcher.ReplacePartialCommand(0xe2da, 0x86, 0xE31A, 6, STR_SPAN("out Nanahoshi Mall"));


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3200

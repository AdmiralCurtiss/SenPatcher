#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t3300[] = "Text fixes in Gen Soba Shop.";
}

namespace SenLib::TX::FileFixesSw::t3300 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t3300.dat",
            95457,
            HyoutaUtils::Hash::SHA1FromHexString("aeb5cf65cf9765fd12bd14f5fbae8511eab1cff0"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Speaking of which, from what I've heard, a pretty rough cold has been going around at
        // Morimiya Academy."
        // This is the first line in a conversation, so the 'Speaking of which' makes no sense.
        // Just remove it.
        // (free time on 7/8 in Chapter 7, before going to Point Q, talk to Manami)
        bin[0x8144 + 5] = 0x46;
        std::swap(bin[0x8152 + 5], bin[0x8162 + 5]);
        std::swap(bin[0x8176 + 5], bin[0x8183 + 5]);
        patcher.RemovePartialCommand(0x812e + 5, 0x124, 0x8131 + 5, 0x13);

        // "The show's host is famous and has\x01a busy schedule, so we don't have\x01a lot of time
        // to work with her."
        // Switch v1.0.1 changed the last 'her' from 'here'.
        // I have no idea who the host is (is it actually a woman?) but this is probably okay as-is.
        // patcher.ReplacePartialCommand(0xb9e3, 0xe2, 0xb9e6, 0x9b, "");


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t3300

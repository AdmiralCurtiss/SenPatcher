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
            HyoutaUtils::Hash::SHA1FromHexString("ea4894524e020bdaed7e8c59971c281001f3420a"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Heh heh... This old man's not going down without a fight, though!"
        // no newline at all here, add one after 'going'
        bin[0x51e0] = 0x01;

        // "With that in mind, we decided it'd be okay to keep the arcade open."
        // This is the first line of a conversation, so there's nothing 'that' could refer to.
        // For reference, this is during the final chapter when the entire town is in the
        // Otherworld, so it's probably saying something like "Despite the situation the town is in,
        // we're keeping the arcade open so people can take their mind off of it."
        // Let's go with...
        static constexpr auto open = STR_SPAN(
            "In spite of the situation, we\x01"
            "decided to keep the arcade open.");
        patcher.ReplacePartialCommand(0x2400, 0x69, 0x2403, 0x43, open);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6100

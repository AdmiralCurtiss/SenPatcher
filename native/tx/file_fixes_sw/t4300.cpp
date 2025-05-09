#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t4300[] = "Text fixes in Le Coultre Antique Shop.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t4300 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t4300.dat",
            104417,
            HyoutaUtils::Hash::SHA1FromHexString("4f7c3e52104a2fbe48f843b28fa689866bd4766f"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // fix the "About Attachments" tip not losing its 'new' icon after reading it
        patcher.RemovePartialCommand(0x572d, 0x1c, 0x5741, 0x5);

        // text colors all look fixed in Switch v1.0.1, yay

        // linebreaks
        // "#E_8#M_9Gonna be meeting everyone\x01at night, but that's it, really."
        std::swap(bin[0x12726], bin[0x12729]);


        // In the International Girl of Mystery quest in the Epilogue (QS_0904_02):
        // The first thing you have to do in this quest is talk to Yukino. Depending on if you have
        // talked to Yukino before during the current NPC dialogue state, you either get her 'hm,
        // not every day you see everyone together, something's up right?' dialogue directly
        // followed by the quest dialogue if you haven't, or you immediately get the quest dialogue
        // if you have. The text flow here is a bit awkward in both situations; whoever edited this
        // didn't realize which line belonged to which conversation. We can fix without too many
        // changes.
        //
        // First, the opener when you've already talked to her. It's "#K#0TTrade secret." because
        // it's written under the assumption that it would follow the line right before it in the
        // script file, which is "#K#0THow do you just KNOW these\x01things?!". We'll actually just
        // swap this with the not-yet-talked variant, where the corresponding line is "#K#0THeh,
        // interesting.", which works way better in this context.
        patcher.ReplacePartialCommand(0x15787, 0x65, 0x1578f, 0xd, STR_SPAN("Heh, interesting."));

        // Then, in the not-yet-talked variant, the conversation goes:
        // "#K#0TI'll be looking forward to your\x01tea when we're back."
        // "#K#0THeehee, likewise. We'll hold you\x01up to that offer."
        // "#K#0THeh, interesting."
        // "#E_0You're actually trying to make\x01contact with the Child of the\x01Otherworld?"
        // "#K#0THow do you just KNOW these\x01things?!"
        // This isn't quite as bad, but still a rather awkward transition. We can change that middle
        // line to something like 'oh wait, before you leave...' and that would work much better.
        patcher.ReplacePartialCommand(
            0x156b7, 0x69, 0x156bf, 0x11, STR_SPAN("Actually, one more thing..."));


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t4300

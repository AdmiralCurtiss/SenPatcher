#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::TX::FileFixesSw::s1100 {
std::string_view GetDescription() {
    return "Text fixes in Kokonoe Shrine (inside).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s1100.dat",
            116121,
            HyoutaUtils::Hash::SHA1FromHexString("a05df3bdd6bc03973235a3af699c6a49d2467053"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);
        using HyoutaUtils::Vector::GetSpan;

        // "Kokonoe School's teachings" -> "Kokonoe School's mindset"
        // This scene has two variants depending on when you visit, and Kou's reply always uses
        // 'mindset' but Sousuke uses two different phrasings, causing a mismatch in one of the
        // variants. Fix this.
        patcher.ReplacePartialCommand(0x17354, 0xbc, 0x173f7, 0x9, GetSpan(bin, 0x17560, 0x7));

        // "I suppose you could say that someone{n}who can match the right wavelength{n}can
        // interfere with matters on the other side."
        // the linebreaks are placed weirdly here, leading to an overhang of the last line, which
        // looks odd. move them.
        std::swap(bin[0x140e2], bin[0x140e6]);
        std::swap(bin[0x14105], bin[0x14109]);


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s1100

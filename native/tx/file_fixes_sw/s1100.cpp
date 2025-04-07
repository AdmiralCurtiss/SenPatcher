#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_s1100[] = "Text fixes in Kokonoe Shrine (inside).";
}

namespace SenLib::TX::FileFixesSw::s1100 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/s1100.dat",
            116121,
            HyoutaUtils::Hash::SHA1FromHexString("bb4d1612616dcfdd6e35fc1660eb89fc461cd74c"));
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
        patcher.ReplacePartialCommand(0x17358, 0xbc, 0x173fb, 0x9, GetSpan(bin, 0x17564, 0x7));

        // "I suppose you could say that someone{n}who can match the right wavelength{n}can
        // interfere with matters on the other side."
        // the linebreaks are placed weirdly here, leading to an overhang of the last line, which
        // looks odd. move them.
        std::swap(bin[0x140e6], bin[0x140ea]);
        std::swap(bin[0x14109], bin[0x1410d]);

        // "I don't have time to be\x01scared of those monster...!"
        patcher.ExtendPartialCommand(0x6f9d, 0x66, 0x6FFD, {{'s'}});


        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::s1100

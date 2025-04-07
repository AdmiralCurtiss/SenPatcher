#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t1110[] = "Text fixes in school 1F.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t1110 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t1110.dat",
            219681,
            HyoutaUtils::Hash::SHA1FromHexString("a3f12bdf8954f235dab36b03ad374cac57f3bdfe"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // Match 3 lines in Jun Memories Menu version to real version:

        // "#4K#F...But, what?"
        // remove comma
        patcher.RemovePartialCommand(0x2ee7d, 0x1c, 0x2EE90, 1);

        // "#E[1]#M_0The days we spent together\x01the real deal. The real us."
        // -> 'are the real deal'
        patcher.ExtendPartialCommand(0x2f6a8, 0x7b, 0x2F705, STR_SPAN(" are"));

        // "#2PMy duty will always remain\x01the same as Seal Knight."
        // -> 'as a Seal Knight'
        patcher.ExtendPartialCommand(0x2f944, 0x40, 0x2F975, STR_SPAN(" a"));

        // "No matter how far apart we may be─#1000W\x01Even if we never see each other again..."
        // this is After Story, near the beginning when you run into Gorou at school
        // broken text speed, harmless
        // patcher.ReplacePartialCommand(0x201f2, 0x5d, 0x2021e, 0x2f, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t1110

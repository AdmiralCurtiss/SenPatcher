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
__declspec(dllexport) char SenPatcherFix_1_ztk_kaoru[] = "Text fixes in conversations with Kaoru.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_kaoru {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_kaoru.dat",
            15713,
            HyoutaUtils::Hash::SHA1FromHexString("c43786412b8b97e8a9e22012030cf8a3c803eef5"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "I've was thinking hard about what\x01I could do to help when it came to me."
        // -> remove the "'ve"
        // patcher.ReplacePartialCommand(0x134b, 0x121, 0x134e, 0x93, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_kaoru

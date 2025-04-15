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
__declspec(dllexport) char SenPatcherFix_1_ztk_hayato[] =
    "Text fixes in conversations with Hayato.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::tk_hayato {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/talk/dat/tk_hayato.dat",
            6615,
            HyoutaUtils::Hash::SHA1FromHexString("1b158bb11188f856c69fc5d323151a26e59ea8d1"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Gah. Yuuji… Getting good grades doesn't mean you can then slack off\x01afterward."
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x9e, 0x102, 0xa1, 0x50, "");
        bin[0xAB] = '.';
        bin[0xAC] = '.';
        bin[0xAD] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::tk_hayato

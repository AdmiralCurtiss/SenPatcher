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
__declspec(dllexport) char SenPatcherFix_1_t6000[] = "Text fixes in Houraichou.";
}

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t6000 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6000.dat",
            258921,
            HyoutaUtils::Hash::SHA1FromHexString("b417be4f5c751e85e5baa11d4d92adccaf51ef56"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "Speakin' of, any reason you're\x01hanging around our offices?"
        // This is written to a follow-up to a part of the conversation you only get if you've
        // previously seen Gotou at the shrine, so the 'speaking of' makes no sense if you don't get
        // that part. Remove that, the line works fine without it.
        std::swap(bin[0x303bc], bin[0x303c4]);
        patcher.ReplacePartialCommand(0x3039b, 0x3f, 0x3039e, 0xe, {{0x41}});

        // "#800WXRC.\x01#3S#800WXanadu Research Club."
        // harmless
        // (EV_06_14_00)
        // patcher.ReplacePartialCommand(0x1af2a, 0x31, 0x1af3f, 0x1a, "");

        // "#3KHama! And Taizou, too…"
        // ellipsis -> three dots
        // patcher.ReplacePartialCommand(0x24b7a, 0x25, 0x24b82, 0x1b, "");
        bin[0x24B9A] = '.';
        bin[0x24B9B] = '.';
        bin[0x24B9C] = '.';

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6000
